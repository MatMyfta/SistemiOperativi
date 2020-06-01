#include "p.h"

#include "../q/q.h"

#define LOG_TAG "p"
#include "../../logger.h"

#include "../../list.h"
#include "../../protocol.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct p_state {
  list *q_list;
  unsigned int m;
};

static void set_m(struct p_state *state) {
  int q_cnt_diff = state->m - list_size(state->q_list);
  unitnos_q *q;
  if (q_cnt_diff > 0) {
    log_debug("Creating %d \"q\"", q_cnt_diff);
    // create new p
    while (q_cnt_diff--) {
      q = unitnos_q_create();
      if (q != NULL) {
        list_push_back(state->q_list, &q);
      }
    }
  } else if (q_cnt_diff < 0) {
    log_debug("Destroying %d \"q\"", -q_cnt_diff);
    // remove p
    while (q_cnt_diff++) {
      list_pop_back(state->q_list);
    }
  }
}

static void add_new_file(struct p_state *state, const char *new_path) {
  list_node *node;
  unitnos_q **q;
  list_for_each_data(q, unitnos_q *, node, state->q_list) {
    unitnos_q_add_new_file(*q, new_path);
  }
}

int unitnos_p_self_main(int in_pipe, int output_pipe) {
  log_debug("Started");

  FILE *fin = fdopen(in_pipe, "r");

  struct p_state state = {0};
  state.q_list = list_create(sizeof(unitnos_q *));

  char *message = NULL;
  size_t message_size = 0;

  while (1) {
    if (getline(&message, &message_size, fin) >= 0) {
      struct unitnos_protocol_command command = unitnos_protocol_parse(message);
      log_verbose("Received command: %s", command.command);

      if (!strcmp(command.command, UNITNOS_P_COMMAND_SET_M)) {
        unsigned int n;
        int ret = sscanf(command.value, "%u", &n);
        assert(ret > 0);
        log_verbose("Received m: %u", n);
        state.m = n;
        set_m(&state);
      }

      if (!strcmp(command.command, UNITNOS_P_COMMAND_ADD_NEW_FILE)) {
        log_verbose("Received file: %s", command.value);
        add_new_file(&state, command.value);
      }
    } else if (feof(fin)) {
      log_debug("Input pipe closed. Terminate");
      break;
    }
  }

  return 0;
}
