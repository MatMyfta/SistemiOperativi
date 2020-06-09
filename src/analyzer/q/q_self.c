#include "q.h"

#define LOG_TAG "q"
#include "../../logger.h"

#include "../../containers/list.h"
#include "../../protocol.h"
#include "../../statistics.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * See
 * https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
 */
#define CEIL_DIV(x, y) (1 + ((x - 1) / y))

struct q_state {
  ssize_t ith;
  ssize_t siblings_cnt;
  int output_pipe;
};

/*******************************************************************************
 * Private functions declarations
 *******************************************************************************/
void send_statistics(struct q_state *state, const char *file);

/*******************************************************************************
 * Public functions implementation
 *******************************************************************************/
int unitnos_q_self_main(int in_pipe, int output_pipe) {
  /*
   * q just need to talk with the parent.
   * The communication can be blocking.
   */
  log_debug("Started");

  FILE *fin = fdopen(in_pipe, "r");

  struct q_state state = {0};
  state.output_pipe = output_pipe;
  // mark as invalid
  state.siblings_cnt = -1;
  state.ith = -1;

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
        /*
         * when the parent sends a new file, it must have already sent ith and
         * siblings_cnt
         */
        assert(state.siblings_cnt > 0);
        assert(state.ith >= 0);
        send_statistics(&state, command.value);
      }

      if (!strcmp(command.command, UNITNOS_Q_COMMAND_CLOSE)) {
        break;
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
void send_statistics(struct q_state *state, const char *file) {
  struct unitnos_char_count_statistics stat = {0};

  int fd = open(file, O_RDONLY);
  if (fd == -1) {
    log_error("Unable to open file \"%s\": %s", file, strerror(errno));
    return;
  }

  off_t file_size = lseek(fd, 0, SEEK_END);
  if (file_size == -1) {
    log_error("Unable to measure size of \"%s\": %s", file, strerror(errno));
    return;
  }
  if (file_size == 0) {
    log_debug("File \"%s\" is empty", file);
  } else {
    lseek(fd, 0, SEEK_SET);

    size_t read_size = file_size / state->siblings_cnt;
    // position cursor at the correct place
    lseek(fd, (read_size * state->ith), SEEK_SET);
    if (state->ith == state->siblings_cnt - 1) {
      read_size = CEIL_DIV(file_size, state->siblings_cnt);
    }
    size_t i;
    char c;
    int ret;
    for (i = 0; i < read_size; ++i) {
      ret = read(fd, &c, 1);
      if (ret == 0) {
        log_warn(
            "Hit unexpected EOF. File has been modified by external software");
        break;
      }
      ++stat.counts[(int)c];
    }
  }
  close(fd);

  log_verbose("Sending statistics for file %s", file);
  unitnos_procotol_send_command_with_data(
      state->output_pipe, getppid(),
      UNITNOS_Q_SELF_COMMAND_SEND_STATISTICS_FILE, "%s", file);
  unitnos_procotol_send_command_with_binary_data(
      state->output_pipe, getppid(),
      UNITNOS_Q_SELF_COMMAND_SEND_STATISTICS_CONTENT, &stat, sizeof(stat));
}
