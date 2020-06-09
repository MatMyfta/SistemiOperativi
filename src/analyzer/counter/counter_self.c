#include "../p/p.h"
#include "counter.h"

#define LOG_TAG "counter"
#include "../../logger.h"

#include "../../containers/dictionary.h"
#include "../../containers/list.h"
#include "../../containers/set.h"
#include "../../protocol.h"
#include "../../utils.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct counter_state {
  unsigned int n;
  unsigned int m;
  /**
   * Counter of received files
   */
  unsigned int files_cnt;
  unitnos_list *unassigned_files;
  /**
   * Keys: process "p"
   * Values: set of path to files
   */
  unitnos_dictionary *p_to_files;
  bool close;
};

/*******************************************************************************
 * Private functions declarations
 *******************************************************************************/
static int dict_key_p_compare(const void *lhs, const void *rhs,
                              void *user_data);
static void dict_key_p_destroy(void *p, void *user_data);
static void dict_value_file_set_destroy(void *file_set, void *user_data);
static void set_n(struct counter_state *state, unsigned int n);
static void set_m(struct counter_state *state, unsigned int m);
static void add_new_file_batch(struct counter_state *state,
                               const char *new_file);
static void add_new_file_batch_finish(struct counter_state *state,
                                      const char *new_file);
/**
 * Terminate all processes "p"
 */
static void terminate_p(struct counter_state *state);

/*******************************************************************************
 * Public functions implementation
 *******************************************************************************/
int unitnos_counter_self_main(int in_pipe, int output_pipe) {
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

  FILE *fin = fdopen(in_pipe, "r");

  struct counter_state state = {0};
  state.p_to_files =
      unitnos_dictionary_create(dict_key_p_compare, dict_key_p_destroy,
                                unitnos_container_util_free, &state);
  state.unassigned_files = unitnos_list_create(
      // don't destroy file paths when removed from the list
      NULL, NULL);

  char *message = NULL;
  size_t message_buf_size = 0;
  ssize_t message_len;

  while (1) {
    unitnos_procotol_wait();

    while ((message_len =
                unitnos_getline(&message, &message_buf_size, in_pipe)) > 0) {
      struct unitnos_protocol_command command = unitnos_protocol_parse(message);
      log_verbose("Received command: %s", command.command);

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_SET_N)) {
        unsigned int n;
        int ret = sscanf(command.value, "%u", &n);
        assert(ret > 0);
        log_verbose("Received n: %u", n);
        set_n(&state, n);
      }

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_SET_M)) {
        unsigned int m;
        int ret = sscanf(command.value, "%u", &m);
        assert(ret > 0);
        log_verbose("Received m: %u", m);
        set_m(&state, m);
      }

      if (!strcmp(command.command,
                  UNITNOS_COUNTER_COMMAND_ADD_NEW_FILE_BATCH)) {
        log_verbose("Received file: %s", command.value);
        add_new_file_batch(&state, command.value);
      }

      if (!strcmp(command.command,
                  UNITNOS_COUNTER_COMMAND_ADD_NEW_FILE_BATCH_FINISH)) {
        log_verbose("Received file: %s", command.value);
        add_new_file_batch_finish(&state, command.value);
      }

      if (!strcmp(command.command, UNITNOS_COUNTER_COMMAND_CLOSE)) {
        state.close = true;
        break;
      }
    }

    if (errno == EAGAIN) {
      log_debug("No message from parent");
    } else if (state.close) {
      break;
    } else if (message_len == 0) {
      log_debug("Input pipe closed. Terminate");
      break;
    } else {
      log_error("Unexpected error %s", strerror(errno));
    }
  }

  terminate_p(&state);

  return 0;
}

/*******************************************************************************
 * Private functions implementation
 *******************************************************************************/
/***************************************
 * container related functions and helpers
 **************************************/
static int dict_key_p_compare(const void *lhs, const void *rhs,
                              void *user_data) {
  if (lhs < rhs) {
    return -1;
  } else if (lhs == rhs) {
    return 0;
  }
  return 1;
}
static void dict_key_p_destroy(void *p, void *user_data) {
  unitnos_p_destroy(p);
}
static bool add_file_to_unassigned(void *value, void *user_data) {
  struct counter_state *state = (struct counter_state *)user_data;
  unitnos_list_push_back(state->unassigned_files, value);
  return false;
}
static void dict_value_file_set_destroy(void *file_set, void *user_data) {
  unitnos_set_foreach(file_set, add_file_to_unassigned, user_data);
  unitnos_set_destroy(file_set);
}

/***************************************
 * container related functions and helpers
 **************************************/
static bool terminate_each_p(void *key, void *value, void *user_data) {
  unitnos_p *p = (unitnos_p *)key;
  struct counter_state *state = (struct counter_state *)user_data;
  unitnos_p_destroy(p);
  return false;
}
static void terminate_p(struct counter_state *state) {
  unitnos_dictionary_foreach(state->p_to_files, terminate_each_p, state);
}

/***************************************
 * set_n, add_new_file and helpers
 **************************************/
struct update_file_assignment_context {
  struct counter_state *state;
  unsigned int files_each_p;
  unsigned int allowed_p_with_one_more_file;
};
static bool update_p_files(void *key, void *value, void *user_data) {
  struct update_file_assignment_context *context =
      (struct update_file_assignment_context *)user_data;
  unitnos_p *p = (unitnos_p *)key;
  unitnos_set *file_set = (unitnos_set *)value;
  size_t files_cnt = unitnos_set_size(file_set);

  size_t allowed_files_cnt = context->files_each_p;
  if (files_cnt < allowed_files_cnt + 1 &&
      context->allowed_p_with_one_more_file > 0) {
    --context->allowed_p_with_one_more_file;
    ++allowed_files_cnt;
  }

  /*
   * For process "p" that has excess of files, put them in the
   * `state->unassigned_files` list.
   * For process "p" that can still have some files, extract them from the
   * `state->unassigned_files` list
   */
  if (files_cnt > allowed_files_cnt) {
    void *assigned_file = NULL;
    while (files_cnt > allowed_files_cnt) {
      assigned_file = unitnos_set_first(file_set);
      unitnos_set_remove(file_set, assigned_file);
      unitnos_list_push_back(context->state->unassigned_files, assigned_file);
      unitnos_p_remove_file(p, assigned_file);
      --files_cnt;
    }
  } else if (files_cnt < allowed_files_cnt) {
    void *unassigned_file = NULL;
    while (files_cnt < allowed_files_cnt &&
           unitnos_list_size(context->state->unassigned_files) > 0) {
      unassigned_file = unitnos_list_back(context->state->unassigned_files);
      unitnos_list_pop_back(context->state->unassigned_files);
      unitnos_set_insert(file_set, unassigned_file);
      unitnos_p_add_new_file(p, unassigned_file);
      ++files_cnt;
    }
  }
  return false;
}
static void update_file_assignment(struct counter_state *state) {
  if (unitnos_dictionary_size(state->p_to_files) == 0) {
    log_debug("There are 0 process \"p\". File assignment won't be performed");
    return;
  }
  struct update_file_assignment_context context;
  context.state = state;
  context.files_each_p =
      state->files_cnt / unitnos_dictionary_size(state->p_to_files);
  context.allowed_p_with_one_more_file =
      state->files_cnt -
      (context.files_each_p * unitnos_dictionary_size(state->p_to_files));
  log_debug("Updating file assignment");
  log_verbose("%u files for each \"p\"", context.files_each_p);
  log_verbose("%u \"p\"s will have 1 more file",
              context.allowed_p_with_one_more_file);
  unitnos_dictionary_foreach(state->p_to_files, update_p_files, &context);
}
static void set_n(struct counter_state *state, unsigned int n) {
  state->n = n;
  int p_cnt_diff = state->n - unitnos_dictionary_size(state->p_to_files);

  unitnos_p *p;
  if (p_cnt_diff > 0) {
    log_debug("Creating %d \"p\"", p_cnt_diff);
    // create new p
    while (p_cnt_diff--) {
      p = unitnos_p_create();
      if (p != NULL) {
        unitnos_set *file_path_set = unitnos_set_create(
            unitnos_container_util_strcmp,
            // don't free file paths when the set is destroyed
            NULL, NULL);
        unitnos_dictionary_insert(state->p_to_files, p, file_path_set);
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
      void *p = unitnos_dictionary_max(state->p_to_files);
      /*
       * p will be destroyed with the appropriate destructor
       *
       * file paths in the file path set associated with the p will be added
       * to the `state->unassigned_files` list.
       */
      unitnos_dictionary_remove(state->p_to_files, p);
    }
  }

  update_file_assignment(state);
}

static void add_new_file_batch(struct counter_state *state,
                               const char *new_file) {
  state->files_cnt++;

  char *str = malloc(strlen(new_file) + 1);
  strcpy(str, new_file);

  unitnos_list_push_back(state->unassigned_files, str);
}

static void add_new_file_batch_finish(struct counter_state *state,
                                      const char *new_file) {
  add_new_file_batch(state, new_file);
  update_file_assignment(state);
}

/***************************************
 * set_m and helpers
 **************************************/
static bool set_m_foreach_p(void *key, void *value, void *user_data) {
  struct counter_state *state = (struct counter_state *)user_data;
  unitnos_p *p = (unitnos_p *)key;
  unitnos_p_set_m(p, state->m);
  return false;
}
static void set_m(struct counter_state *state, unsigned int m) {
  state->m = m;
  log_debug("Updating m of %lu \"p\"",
            unitnos_dictionary_size(state->p_to_files));
  unitnos_dictionary_foreach(state->p_to_files, set_m_foreach_p, state);
}
