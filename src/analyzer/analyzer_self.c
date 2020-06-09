#include "analyzer.h"

#include "counter/counter.h"
#include "path.h"

#define LOG_TAG "analyzer"
#include "../logger.h"

#include "../containers/dictionary.h"
#include "../containers/list.h"
#include "../protocol.h"
#include "../statistics.h"
#include "../utils.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h> 

#define unitnos_fifo "/tmp/unitnos_fifo"
#define ack_end "ACK_END"
#define MAX_SIZE_MESSAGE 100

struct analyzer_state {
  unitnos_counter *counter;
  /**
   * * Keys: paths to either directory of files
   * * Values: dictionary
   *     * Keys: paths to files
   *     * Values: unitnos_char_count_statistics
   */
  unitnos_dictionary *statistics;
  bool close;
};

/*******************************************************************************
 * Private functions declarations
 *******************************************************************************/
static int dict_path_compare(const void *lhs, const void *rhs, void *user_data);
static void dict_value_dict_destroy(void *dict, void *user_data);

static void add_new_path(struct analyzer_state *state, const char *new_path);
static void list_paths(struct analyzer_state *state);
/**
 * Process messages from child "counter" process_counter
 */
static void process_counter(struct analyzer_state *state);

/**
 * Function for communication through FIFO with report
 */
static void command_global(struct unitnos_protocol_command command);
static void command_path(struct unitnos_protocol_command command);
static int send_message(char* message_to_send);
static char* read_message(int size);
static char* concat(const char *s1, const char *s2);

/*******************************************************************************
 * Public functions implementation
 *******************************************************************************/
int unitnos_analyzer_self_main(int in_pipe, int output_pipe) {
  log_debug("Started");

  if (unitnos_procotol_init() == -1) {
    log_error("Unable to initialize communication protocol");
    exit(-1);
  }

  unitnos_process_init(in_pipe, output_pipe);

  if (unitnos_set_non_blocking(in_pipe)) {
    log_error("Unable to set input pipe to non-blocking mode");
    exit(-1);
  }

  struct analyzer_state state = {0};
  state.counter = unitnos_counter_create();
  if (!state.counter) {
    log_error("Unable to create \"counter\" process");
    return -1;
  }

  state.statistics =
      unitnos_dictionary_create(dict_path_compare, unitnos_container_util_free,
                                dict_value_dict_destroy, NULL);

  char *message = NULL;
  size_t message_buf_size = 0;
  ssize_t message_len = 0;

  int fd;
  mkfifo(unitnos_fifo, 0666);
  char received[MAX_SIZE_MESSAGE];
  ssize_t nread;
  fd = open(unitnos_fifo, O_RDONLY | O_NONBLOCK);

  while (1) {
    unitnos_procotol_wait();

    /**
     *lettura non bloccante su FIFO, se trova COMANDO avvia operazioni bloccanti
     */
    nread=read(fd, received, 80);
    switch (nread) {
      case -1: {
        if (errno == EAGAIN) {
          log_verbose("%s\n", "no message");
          sleep(1);
          break;
        } else {
          log_error("%s\n", "Error with reading");
          exit(1);
        }; break;
      }
      case 0: {
        log_verbose("%s\n", "EOF reached");
      }; break;
      default: {
        close(fd);
        struct unitnos_protocol_command command = unitnos_protocol_parse(received);
        if (strcmp(command.command,"global")==0)
          command_global(command);
        if (strcmp(command.command,"path")==0)
          command_path(command);
        fd = open(unitnos_fifo, O_RDONLY | O_NONBLOCK);
      }; break;
    }

    process_counter(&state);

    while ((message_len =
                unitnos_getline(&message, &message_buf_size, in_pipe)) > 0) {
      struct unitnos_protocol_command command = unitnos_protocol_parse(message);
      log_verbose("Received command: %s", command.command);

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_SET_N)) {
        unsigned int n;
        int ret = sscanf(command.value, "%u", &n);
        assert(ret > 0);
        unitnos_counter_set_n(state.counter, n);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_SET_M)) {
        unsigned int m;
        int ret = sscanf(command.value, "%u", &m);
        assert(ret > 0);
        unitnos_counter_set_m(state.counter, m);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_ADD_NEW_PATH)) {
        add_new_path(&state, command.value);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_LIST_PATHS)) {
        list_paths(&state);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_STATUS_PANEL)) {
        printf("Analyzer at PID %u\n", getpid());
        unitnos_counter_status_panel(state.counter);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_CLOSE)) {
        state.close = true;
        break;
      }
    }

    if (state.close) {
      break;
    }

    if (message_len == 0) {
      log_debug("Input pipe closed. Terminate");
      break;
    }

    if (errno == EAGAIN) {
      log_debug("No message from parent");
    } else {
      log_error("Unexpected error %s", strerror(errno));
    }
  }
  unitnos_counter_delete(state.counter);

  return 0;
}

/*******************************************************************************
 * Private functions implementation
 *******************************************************************************/
static int dict_path_compare(const void *lhs, const void *rhs,
                             void *user_data) {
  return strcmp(lhs, rhs);
}
static void dict_value_dict_destroy(void *dict, void *user_data) {
  unitnos_dictionary_destroy(dict);
}
/**************************************
 * process_counter and helpers
 *************************************/
struct find_path_of_file_context {
  const char *found_path;
  const char *file;
};
static bool find_path_of_file(void *key, void *value, void *user_data) {
  const char *path = (const char *)key;
  struct find_path_of_file_context *context =
      (struct find_path_of_file_context *)user_data;

  if (strstr(context->file, path) == context->file) {
    context->found_path = path;
    return true;
  }
  return false;
}
static void on_new_statistics(unitnos_counter *counter, const char *file,
                              struct unitnos_char_count_statistics *statistics,
                              void *user_data) {
  log_debug("Received statistics for file %s", file);
  struct analyzer_state *state = (struct analyzer_state *)user_data;

  struct unitnos_char_count_statistics *stat =
      unitnos_malloc(sizeof(struct unitnos_char_count_statistics));
  memcpy(stat, statistics, sizeof(struct unitnos_char_count_statistics));

  // find path associated with file
  struct find_path_of_file_context context;
  context.found_path = NULL;
  context.file = file;
  unitnos_dictionary_foreach(state->statistics, find_path_of_file, &context);
  assert(context.found_path != NULL);

  // insert stat into dictionary
  unitnos_dictionary *file_stat_dict =
      unitnos_dictionary_lookup(state->statistics, context.found_path);
  assert(file_stat_dict != NULL);
  unitnos_dictionary_insert(file_stat_dict, (void *)file, stat);

  log_info("Received statistics for file %s", file);
}
static void process_counter(struct analyzer_state *state) {
  struct unitnos_counter_event_callbacks cbs;
  cbs.on_new_statistics = on_new_statistics;
  unitnos_counter_process(state->counter, cbs, state);
}
/**************************************
 * add_new_path and helpers
 *************************************/
static bool insert_into_dict(void *file, void *user_data) {
  unitnos_dictionary *file_statistics_dict = (unitnos_dictionary *)user_data;
  unitnos_dictionary_insert(file_statistics_dict, file, NULL);
  return false;
}
static void add_new_path(struct analyzer_state *state, const char *new_path) {
  // normalize path
  char *normalized_path = realpath(new_path, NULL);
  if (!normalized_path) {
    log_error("Invalid path \"%s\": %s", new_path, strerror(errno));
    return;
  }
  if (unitnos_dictionary_contains(state->statistics, normalized_path)) {
    log_info("Path already exists");
    free(normalized_path);
    return;
  }

  unitnos_set *file_paths_set = unitnos_unpack_path(normalized_path);
  if (file_paths_set) {
    log_info("The new added path contains %lu files",
             unitnos_set_size(file_paths_set));
    unitnos_dictionary *dict =
        unitnos_dictionary_create(unitnos_container_util_strcmp,
                                  /*
                                   * file_paths_set doesn't free the values'.
                                   * The dictionary will take care to free them.
                                   */
                                  unitnos_container_util_free,
                                  /*
                                   * Free statistics
                                   */
                                  unitnos_container_util_free, NULL);
    unitnos_set_foreach(file_paths_set, insert_into_dict, dict);
    unitnos_counter_add_new_files_batch(state->counter, file_paths_set);
    unitnos_dictionary_insert(state->statistics, normalized_path, dict);
    unitnos_set_destroy(file_paths_set);
  }
}

/**************************************
 * list_paths and helpers
 *************************************/
struct list_file_paths_callback_context {
  const char *directory_path;
  size_t index;
  size_t size;
};
static bool list_file_paths_callback(void *key, void *value, void *user_data) {
  struct list_file_paths_callback_context *context =
      (struct list_file_paths_callback_context *)user_data;
  const char *file_path = (const char *)key;
  if (context->index == context->size - 1) {
    printf("└");
  } else {
    printf("├");
  }
  printf("── %s\n", file_path + strlen(context->directory_path) + 1);
  context->index++;
  return false;
}
static bool list_paths_callback(void *key, void *value, void *user_data) {
  const char *path = (const char *)key;
  unitnos_dictionary *file_statistics_dict = (unitnos_dictionary *)value;
  printf("Path: %s\n", path);
  struct list_file_paths_callback_context context;
  context.directory_path = path;
  context.index = 0;
  context.size = unitnos_dictionary_size(file_statistics_dict);
  unitnos_dictionary_foreach(file_statistics_dict, list_file_paths_callback,
                             &context);
  return false;
}
static void list_paths(struct analyzer_state *state) {
  unitnos_dictionary_foreach(state->statistics, list_paths_callback, NULL);
}

/**************************************
 * Communication trough pipe and helpers
 *************************************/
static void command_global(struct unitnos_protocol_command command) {
  // DA DEFINIRE LA STRINGA
  send_message(concat("STATISTICA globale - type: ",command.value));
}
static void command_path(struct unitnos_protocol_command command) {
    char* path = "prova_path.txt";
    char* response;
    log_verbose("%s\n", "Command_path requested");
    int numero_path = 4, i = 0;
    while ((i>=0) && (i<numero_path)) {
        send_message(concat("STATISTICA per path:", path));
        response = read_message(MAX_SIZE_MESSAGE);
        if (response!=NULL) {
            struct unitnos_protocol_command second_command = unitnos_protocol_parse(response);
            if ((strcmp(second_command.command, "ack")==0) && (strcmp(second_command.value,"1")==0)) { //lazy evaluation
                send_message(concat("VALORE STATISTICA", command.value));
                response = read_message(MAX_SIZE_MESSAGE);
                if (response!=NULL) {
                    struct unitnos_protocol_command third_command = unitnos_protocol_parse(response);
                    if ((strcmp(second_command.command, "ack")==0) && (strcmp(third_command.value,"1")==0)) //lazy evaluation
                        i++;
                    else {
                        send_message(ack_end);
                        i=-1;
                    }
                } else {
                    send_message(ack_end);
                    i=-1;
                }
            } else {
                send_message(ack_end);
                i=-1;
            }
        } else {
            send_message(ack_end);
            i=-1;
        }
    }
    send_message(ack_end);
}
static int send_message(char *message_to_send) {
    int fd, ret_value;
    fd = open(unitnos_fifo, O_WRONLY);
    if (fd==-1) {
        log_error("Failed opening named pipe");
        ret_value = 1;
    } else {
        if (write(fd, message_to_send, strlen(message_to_send)+1)==-1) {
            log_error("Failed writing on named pipe");
            ret_value = 1;
        } else {
            ret_value = 0;
        }
        close(fd);
    }
    return ret_value;
}
static char* read_message(int size) {
    char* ret_value;
    int fd;
    ssize_t readed;
    char *message_to_read = malloc(size +1);
    fd = open(unitnos_fifo, O_RDONLY);
    if (fd==-1) {
        log_error("Failed opening named pipe");
        free(message_to_read);
        ret_value=NULL;
    } else {
        readed=read(fd, message_to_read, size +1);
        if (readed==-1) {
            log_error("Failed writing on named pipe");
            free(message_to_read);
            ret_value=NULL;
        } else {
            message_to_read[readed]='\0';
            ret_value = message_to_read;
        }
        close(fd);
    }
    return ret_value;  
}

static char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2); 
    return result;
}
