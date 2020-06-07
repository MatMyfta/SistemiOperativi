#include "q.h"

#define LOG_TAG "q"
#include "../../logger.h"

#include "../../containers/list.h"
#include "../../protocol.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct q_state {
  unsigned int ith;
  unsigned int siblings_cnt;
};

int unitnos_q_self_main(int in_pipe, int output_pipe) {
  log_debug("Started");

  FILE *fin = fdopen(in_pipe, "r");

  struct q_state state = {0};

  char *message = NULL;
  size_t message_size = 0;

  while (1) {
    if (getline(&message, &message_size, fin) >= 0) {
      struct unitnos_protocol_command command = unitnos_protocol_parse(message);
      log_verbose("Received command: %s", command.command);

      if (!strcmp(command.command, UNITNOS_Q_COMMAND_SET_ITH)) {
        unsigned int ith;
        int ret = sscanf(command.value, "%u", &ith);
        assert(ret > 0);
        log_verbose("Received ith: %u", ith);
        state.ith = ith;
      }

      if (!strcmp(command.command, UNITNOS_Q_COMMAND_SET_SIBLINGS_CNT)) {
        unsigned int siblings_cnt;
        int ret = sscanf(command.value, "%u", &siblings_cnt);
        assert(ret > 0);
        log_verbose("Received siblings_cnt: %u", siblings_cnt);
        state.siblings_cnt = siblings_cnt;
      }

      if (!strcmp(command.command, UNITNOS_Q_COMMAND_ADD_NEW_FILE)) {
        log_verbose("Received file: %s", command.value);
      }
    } else if (feof(fin)) {
      log_debug("Input pipe closed. Terminate");
      break;
    }
  }

  return 0;
}
