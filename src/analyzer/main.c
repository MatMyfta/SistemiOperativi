#include "analyzer.h"

#include "counter/counter.h"

#define LOG_TAG "analyzer"
#include "../logger.h"

#include "../process.h"
#include "../protocol.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h> 
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>

const char * unitnos_fifo = "/tmp/unitnos_fifo";
char *ack_end = "ACK_END";
const int MAX_SIZE_MESSAGE=100;

static int independent_analyzer_main(int argc, char **argv);
static int child_analyzer_main(int in_pipe, int output_pipe);

static void command_global(struct unitnos_protocol_command command);
static void command_path(struct unitnos_protocol_command command);
static int send_ack(char* value);
static int send_message(char* message_to_send);
static char* read_message(int size);
static char* concat(const char *s1, const char *s2);

int main(int argc, char *argv[]) {
  if (unitnos_process_is_process(argc, argv)) {
    return child_analyzer_main(atoi(argv[1]), atoi(argv[2]));
  } else {
    return independent_analyzer_main(argc, argv);
  }
}

static int independent_analyzer_main(int argc, char **argv) {
  log_debug("Analyzer running in standalone mode");
  if (argc < 4) {
    log_error("Usage: analyzer N M PATH...");
    return -1;
  }

  unitnos_counter *counter = unitnos_counter_create();
  if (!counter) {
    log_error("Unable to create counter");
    return -1;
  }

  unitnos_counter_set_n(counter, atoi(argv[1]));
  unitnos_counter_set_m(counter, atoi(argv[2]));
  {
    int i;
    for (i = 3; i < argc; ++i) {
      unitnos_counter_add_new_path(counter, argv[i]);
    }
  }

  int fd;
  mkfifo(unitnos_fifo, 0666);
  char received[80];
  ssize_t nread;
  fd = open(unitnos_fifo, O_RDONLY | O_NONBLOCK);

  while (1) {
    nread=read(fd, received, 80);
    switch (nread) {
      case -1: {
        if (errno == EAGAIN) {
          break;
        } else {
          log_error("%s\n", "Error with reading");
          exit(1);
        }; break;
      }
      case 0: {
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
    //unitnos_counter_process(counter);
  }
  unitnos_counter_delete(counter);
  return 0;
}

static int child_analyzer_main(int in_pipe, int output_pipe) {
  log_debug("Analyzer running in child mode");

  FILE *fin = fdopen(in_pipe, "r");

  unitnos_counter *counter = unitnos_counter_create();
  if (!counter) {
    log_error("Unable to create counter");
    return -1;
  }

  char *message = NULL;
  size_t message_size = 0;

  while (1) {
    if (getline(&message, &message_size, fin) >= 0) {
      struct unitnos_protocol_command command = unitnos_protocol_parse(message);
      log_verbose("Received command: %s", command.command);

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_SET_N)) {
        unsigned int n;
        int ret = sscanf(command.value, "%u", &n);
        assert(ret > 0);
        unitnos_counter_set_n(counter, n);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_SET_M)) {
        unsigned int m;
        int ret = sscanf(command.value, "%u", &m);
        assert(ret > 0);
        unitnos_counter_set_m(counter, m);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_ADD_NEW_PATH)) {
        unitnos_counter_add_new_path(counter, command.value);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_LIST_PATHS)) {
        unitnos_counter_list_paths(counter);
      }
    } else if (feof(fin)) {
      log_debug("Input pipe closed. Terminate");
      unitnos_counter_delete(counter);
      break;
    }
  }

  return 0;
}

static void command_global(struct unitnos_protocol_command command) {
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

struct unitnos_protocol_command unitnos_protocol_parse(char *message) {
  struct unitnos_protocol_command command;
  command.command = message;

  char *newl = strchr(message, '\n');
  assert(newl != NULL);
  *newl = '\0';
  char *delim = strchr(message, ':');
  if (delim) {
    command.value = delim + 1;
    *delim = '\0';
  }

  return command;
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
