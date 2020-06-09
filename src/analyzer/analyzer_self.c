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

struct analyzer_state {
  unitnos_counter *counter;
  /**
   * * Keys: paths to either directory of files
   * * Values: dictionary
   *     * Keys: paths to files
   *     * Values: unitnos_char_count_statistics
   */
  unitnos_dictionary *statistics;
};

/*******************************************************************************
 * Private functions declarations
 *******************************************************************************/
static int dict_path_compare(const void *lhs, const void *rhs, void *user_data);
static void dict_value_dict_destroy(void *dict, void *user_data);

static void add_new_path(struct analyzer_state *state, const char *new_path);
static void list_paths(struct analyzer_state *state);

/*******************************************************************************
 * Public functions implementation
 *******************************************************************************/
int unitnos_analyzer_self_main(int in_pipe, int output_pipe) {
  log_debug("Started");

  if (unitnos_procotol_init() == -1) {
    log_error("Unable to initialize communication protocol");
    exit(-1);
  }

  if (unitnos_set_non_blocking(in_pipe)) {
    log_error("Unable to set input pipe to non-blocking mode");
    exit(-1);
  }

  FILE *fin = fdopen(in_pipe, "r");

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
  size_t message_size = 0;

  while (1) {
    unitnos_procotol_wait();

    if (getline(&message, &message_size, fin) >= 0) {
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

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_CLOSE)) {
        break;
      }
    } else if (feof(fin)) {
      log_debug("Input pipe closed. Terminate");
      break;
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
