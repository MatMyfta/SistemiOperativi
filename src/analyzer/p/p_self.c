#include "p.h"

#include "../q/q.h"

#define LOG_TAG "p"
#include "../../logger.h"

#include "../../containers/dictionary.h"
#include "../../containers/list.h"
#include "../../protocol.h"
#include "../../statistics.h"
#include "../../utils.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct file_statistics {
  /**
   * The final whole-file statistics in which statistics of file chunks from
   * "q" are gradually merged.
   */
  struct unitnos_char_count_statistics statistics;
  /**
   * The number of process "q" that haven't supplied their portion of
   * statistics yet
   */
  unsigned int missing;
};

struct p_state {
  unitnos_list *q_list;
  /**
   * Keys: file paths
   * Values: instance of `struct file_statistics`
   */
  unitnos_dictionary *file_statistics_dict;
  unsigned int m;
  int output_pipe;
  bool close;
};

/*******************************************************************************
 * Private functions declarations
 *******************************************************************************/
static void q_destructor(void *value, void *user_data);
static void set_m(struct p_state *state, unsigned int m);
static void add_new_file(struct p_state *state, const char *new_file);
static void remove_file(struct p_state *state, const char *removed_file);
/**
 * Process any message/commands from child processes "q"
 */
static void process_q(struct p_state *state);
/**
 * Terminate all processes "q"
 */
static void terminate_q(struct p_state *state);
/**
 * Print status of each child process "q"
 */
static void q_status(struct p_state *state);

/*******************************************************************************
 * Public functions implementation
 *******************************************************************************/
static sig_atomic_t g_sigterm_received = 0;
static void sigterm(int signo) { g_sigterm_received++; }

int unitnos_p_self_main(int in_pipe, int output_pipe) {
  log_debug("Started");

  if (signal(SIGTERM, sigterm) == SIG_ERR) {
    log_error("Unable to register SIGTERM handler");
    exit(-1);
  }

  if (unitnos_procotol_init() == -1) {
    log_error("Unable to initialize communication protocol");
    exit(-1);
  }

  unitnos_process_init(in_pipe, output_pipe);

  if (unitnos_set_non_blocking(in_pipe)) {
    log_error("Unable to set input pipe to non-blocking mode");
    exit(-1);
  }

  struct p_state state = {0};
  state.output_pipe = output_pipe;
  state.q_list = unitnos_list_create(q_destructor, &state);
  state.file_statistics_dict = unitnos_dictionary_create(
      unitnos_container_util_strcmp, unitnos_container_util_free,
      unitnos_container_util_free, NULL);

  char *message = NULL;
  size_t message_buf_size = 0;
  ssize_t message_len = 0;

  while (!g_sigterm_received) {
    unitnos_procotol_wait();

    process_q(&state);

    while ((message_len =
                unitnos_getline(&message, &message_buf_size, in_pipe)) > 0) {
      struct unitnos_protocol_command command = unitnos_protocol_parse(message);
      log_verbose("Received command: %s", command.command);

      if (!strcmp(command.command, UNITNOS_P_COMMAND_SET_M)) {
        unsigned int m;
        int ret = sscanf(command.value, "%u", &m);
        assert(ret > 0);
        log_verbose("Received m: %u", m);
        set_m(&state, m);
      }

      if (!strcmp(command.command, UNITNOS_P_COMMAND_ADD_NEW_FILE)) {
        log_verbose("Received file: %s", command.value);
        add_new_file(&state, command.value);
        process_q(&state);
      }

      if (!strcmp(command.command, UNITNOS_P_COMMAND_REMOVE_FILE)) {
        log_verbose("Received file: %s", command.value);
        remove_file(&state, command.value);
      }

      if (!strcmp(command.command, UNITNOS_P_COMMAND_STATUS)) {
        q_status(&state);
      }

      if (!strcmp(command.command, UNITNOS_P_COMMAND_CLOSE)) {
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

  terminate_q(&state);

  return 0;
}

/*******************************************************************************
 * Private functions implementation
 *******************************************************************************/
static void q_destructor(void *value, void *user_data) {
  unitnos_q_destroy(value);
}

/*****************************************
 * process_q and helpers
 ****************************************/
static void on_new_statistics(unitnos_q *q, const char *file,
                              struct unitnos_char_count_statistics *statistics,
                              void *user_data) {
  log_debug("Received partial statistics for file %s", file);
  struct p_state *state = (struct p_state *)user_data;

  struct file_statistics *stat =
      unitnos_dictionary_lookup(state->file_statistics_dict, file);
  /*
   * If there're no bugs in p and q, the file must exist in the dictionary
   * and there should be at least one partial file statistics missing.
   */
  assert(stat);
  assert(stat->missing != 0);
  stat->missing--;

  // merge new partial statistics into whole-file statistics
  size_t i;
  for (i = 0;
       i < sizeof(stat->statistics.counts) / sizeof(stat->statistics.counts[0]);
       ++i) {
    stat->statistics.counts[i] += statistics->counts[i];
  }

  if (stat->missing == 0) {
    log_debug("Statistics completed for file %s", file);
    unitnos_procotol_send_command_with_data(
        state->output_pipe, getppid(),
        UNITNOS_P_SELF_COMMAND_SEND_STATISTICS_FILE, "%s", file);
    unitnos_procotol_send_command_with_binary_data(
        state->output_pipe, getppid(),
        UNITNOS_P_SELF_COMMAND_SEND_STATISTICS_CONTENT, &stat->statistics,
        sizeof(stat->statistics));
  }
}
static bool process_each_q(void *value, void *user_data) {
  unitnos_q *q = (unitnos_q *)value;
  struct p_state *state = (struct p_state *)user_data;
  struct unitnos_q_event_callbacks cbs;
  cbs.on_new_statistics = on_new_statistics;
  unitnos_q_process(value, cbs, state);
  return false;
}
static void process_q(struct p_state *state) {
  unitnos_list_foreach(state->q_list, process_each_q, state);
}

/*****************************************
 * terminate_q and helpers
 ****************************************/
static bool terminate_each_q(void *value, void *user_data) {
  unitnos_q *q = (unitnos_q *)value;
  unitnos_q_destroy(q);
  return false;
}
static void terminate_q(struct p_state *state) {
  unitnos_list_foreach(state->q_list, terminate_each_q, state);
}

/*******************************************************************************
 * set_m and helpers
 *******************************************************************************/
struct set_m_context {
  struct p_state *state;
  unsigned int new_m;
  unsigned int old_m;
  size_t index;
};
static bool send_file_to_new_q(void *key, void *value, void *user_data) {
  const char *file = (const char *)key;
  unitnos_q *q = (unitnos_q *)user_data;
  unitnos_q_add_new_file(q, file);
  return false;
}
static bool send_new_m(void *value, void *user_data) {
  struct set_m_context *context = (struct set_m_context *)user_data;
  unitnos_q *q = (unitnos_q *)value;

  unitnos_q_set_siblings_cnt(q, context->new_m);

  /*
   * We add and remove the list of "q" in LIFO.
   * For "q"s that are not newly created we don't have to update their index
   * and the files they have to read
   */
  if (context->index >= context->old_m) {
    unitnos_q_set_ith(q, context->index);
    unitnos_dictionary_foreach(context->state->file_statistics_dict,
                               send_file_to_new_q, q);
  }

  ++context->index;
  return false;
}

static bool invalidate_statistics(void *key, void *value, void *user_data) {
  struct p_state *state = (struct p_state *)user_data;
  struct file_statistics *stat = (struct file_statistics *)value;
  memset(stat, 0, sizeof(struct file_statistics));
  stat->missing = state->m;
  return false;
}

static void set_m(struct p_state *state, unsigned int m) {
  struct set_m_context context;
  context.old_m = state->m;
  context.new_m = m;
  context.state = state;
  context.index = 0;

  state->m = m;
  int q_cnt_diff = state->m - unitnos_list_size(state->q_list);

  unitnos_q *q;
  if (q_cnt_diff > 0) {
    log_debug("Creating %d \"q\"", q_cnt_diff);
    // create new q
    while (q_cnt_diff--) {
      q = unitnos_q_create();
      if (q != NULL) {
        unitnos_list_push_back(state->q_list, q);
      }
    }
  } else if (q_cnt_diff < 0) {
    log_debug("Destroying %d \"q\"", -q_cnt_diff);
    // remove q
    while (q_cnt_diff++) {
      unitnos_list_pop_back(state->q_list);
    }
  }

  // m has changed. Invalidate the statistics.
  unitnos_dictionary_foreach(state->file_statistics_dict, invalidate_statistics,
                             state);
  unitnos_list_foreach(state->q_list, send_new_m, &context);
}

/*******************************************************************************
 * add_new_file and helpers
 *******************************************************************************/
static bool send_new_path(void *value, void *user_data) {
  const char *file = (const char *)user_data;
  unitnos_q *q = (unitnos_q *)value;
  unitnos_q_add_new_file(q, file);
  return false;
}
static void add_new_file(struct p_state *state, const char *new_file) {
  char *str = unitnos_malloc(strlen(new_file) + 1);
  strcpy(str, new_file);

  struct file_statistics *stat = unitnos_malloc(sizeof(struct file_statistics));
  memset(stat, 0, sizeof(struct file_statistics));
  stat->missing = state->m;

  unitnos_dictionary_insert(state->file_statistics_dict, str, stat);
  unitnos_list_foreach(state->q_list, send_new_path, str);
}

/*******************************************************************************
 * remove_file and helpers
 *******************************************************************************/
static bool remove_foreach_q(void *value, void *user_data) {
  const char *removed_file = (const char *)user_data;
  unitnos_q *q = (unitnos_q *)value;
  unitnos_q_remove_file(q, removed_file);
  return false;
}
static void remove_file(struct p_state *state, const char *removed_file) {
  unitnos_dictionary_remove(state->file_statistics_dict, removed_file);
  unitnos_list_foreach(state->q_list, remove_foreach_q, (void *)removed_file);
}

/*******************************************************************************
 * q_status and helpers
 *******************************************************************************/
static bool status_foreach_q(void *value, void *user_data) {
  struct p_state *state = (struct p_state *)user_data;
  unitnos_q *q = (unitnos_q *)value;
  printf("\t\t\tQ PID %u, created by P PID %u\n", unitnos_q_get_pid(q),
         getpid());
  return false;
}

static void q_status(struct p_state *state) {
  unitnos_list_foreach(state->q_list, status_foreach_q, state);
}
