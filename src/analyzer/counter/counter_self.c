#include "../p/p.h"
#include "counter.h"
#include "path.h"

#define LOG_TAG "counter"
#include "../../logger.h"

#include "../../containers/dictionary.h"
#include "../../containers/list.h"
#include "../../protocol.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct counter_state {
  list *p_list;
  unsigned int n;
  unsigned int m;
  /**
   * Dictionary that uses path to either directory or file as keys and an array
   * of paths to files as values.
   */
  unitnos_dictionary *path_files_map;
};

/*******************************************************************************
 * Private functions declarations
 *******************************************************************************/
static void util_set_destroy(void *set, void *user_data);
static void set_n(struct counter_state *state);
static void set_m(struct counter_state *state);
static void add_new_path(struct counter_state *state, const char *new_path);
static void list_paths(struct counter_state *state);
static void status_panel(struct counter_state *state);

/*******************************************************************************
 * Public functions implementation
 *******************************************************************************/
int unitnos_counter_self_main(int in_pipe, int output_pipe) {
  log_debug("Started");

  FILE *fin = fdopen(in_pipe, "r");

  struct counter_state state = {0};
  state.p_list = list_create(sizeof(unitnos_p *));
  state.path_files_map = unitnos_dictionary_create(
      unitnos_container_util_strcmp, unitnos_container_util_free,
      util_set_destroy, NULL);

  char *message = NULL;
  size_t message_size = 0;

  while (1) {
    if (getline(&message, &message_size, fin) >= 0) {
      struct unitnos_protocol_command command = unitnos_protocol_parse(message);
      log_verbose("Received command: %s", command.command);

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_SET_N)) {
        unsigned int n;
        int ret = sscanf(command.value, "%u", &n);
        assert(ret > 0);
        log_verbose("Received n: %u", n);
        state.n = n;
        set_n(&state);
      }

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_SET_M)) {
        unsigned int m;
        int ret = sscanf(command.value, "%u", &m);
        assert(ret > 0);
        log_verbose("Received m: %u", m);
        state.m = m;
        set_m(&state);
      }

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_ADD_NEW_PATH)) {
        log_verbose("Received path: %s", command.value);
        add_new_path(&state, command.value);
      }

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_LIST_PATHS)) {
        list_paths(&state);
      }

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_STATUS_PANEL)) {
        status_panel(&state);
      }
    } else if (feof(fin)) {
      log_debug("Input pipe closed. Terminate");
      break;
    }
  }

  return 0;
}

/*******************************************************************************
 * Private functions implementation
 *******************************************************************************/
static void util_set_destroy(void *set, void *user_data) {
  unitnos_set_destroy((unitnos_set *)set);
}

static void set_n(struct counter_state *state) {
  int p_cnt_diff = state->n - list_size(state->p_list);
  unitnos_p *p;
  if (p_cnt_diff > 0) {
    log_debug("Creating %d \"p\"", p_cnt_diff);
    // create new p
    while (p_cnt_diff--) {
      p = unitnos_p_create();
      if (p != NULL) {
        list_push_back(state->p_list, &p);
        if (state->m != 0) {
          unitnos_p_set_m(p, state->m);
        }
      }
    }
  } else if (p_cnt_diff < 0) {
    log_debug("Destroying %d \"p\"", -p_cnt_diff);
    unitnos_p **p;
    // remove p
    while (p_cnt_diff++) {
      p = list_back(state->p_list);
      unitnos_p_destroy(*p);
      list_pop_back(state->p_list);
    }
  }
}

static void set_m(struct counter_state *state) {
  list_node *node;
  unitnos_p **p;
  log_debug("Updating m of %lu \"p\"", list_size(state->p_list));
  list_for_each_data(p, unitnos_p *, node, state->p_list) {
    unitnos_p_set_m(*p, state->m);
  }
}

static void add_new_path(struct counter_state *state, const char *new_path) {
  // normalize path
  char *normalized_path = realpath(new_path, NULL);
  if (!normalized_path) {
    log_error("Invalid path \"%s\": %s", new_path, strerror(errno));
    return;
  }
  if (unitnos_dictionary_contains(state->path_files_map, normalized_path)) {
    log_info("Path already exists");
    free(normalized_path);
    return;
  }

  unitnos_set *paths_set = unitnos_unpack_path(normalized_path);
  if (paths_set) {
    log_info("The new added path contains %lu files",
             unitnos_set_size(paths_set));
    unitnos_dictionary_insert(state->path_files_map, normalized_path,
                              paths_set);
  }
}

struct list_file_paths_callback_context {
  const char *directory_path;
  size_t index;
  size_t size;
};
static bool list_file_paths_callback(void *value, void *user_data) {
  struct list_file_paths_callback_context *context =
      (struct list_file_paths_callback_context *)user_data;
  const char *file_path = (const char *)value;
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
  unitnos_set *file_paths_set = (unitnos_set *)value;
  printf("Path: %s\n", path);
  struct list_file_paths_callback_context context;
  context.directory_path = path;
  context.index = 0;
  context.size = unitnos_set_size(file_paths_set);
  unitnos_set_foreach(file_paths_set, list_file_paths_callback, &context);
  return false;
}
static void list_paths(struct counter_state *state) {
  unitnos_dictionary_foreach(state->path_files_map, list_paths_callback, NULL);
}
static void status_panel(struct counter_state *state) {
  list_node *node_p;
  unitnos_p **p;
  list_for_each_data(p, unitnos_p *, node_p, state->p_list) {
    sleep(1); //OVVIAMENTE INGUARDABILE, TEMPORANEO SOLO PER CONTROLLO RISULTATI
    log_info("P PID %u\n",unitnos_p_get_pid(*p));
    unitnos_p_status(*p);
    }
}
