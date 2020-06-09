#include "q.h"

#include "../../process.h"
#include "../../protocol.h"
#include "../../utils.h"

#define LOG_TAG "q_qarent"
#include "../../logger.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct unitnos_q {
  unitnos_process *process;
};

unitnos_q *unitnos_q_create(void) {
  char *const argv[] = {(char *)NULL};
  unitnos_process *process = unitnos_program_open(UNITNOS_PROGRAM_Q, argv);
  if (!process) {
    return NULL;
  } else {
    unitnos_q *p = malloc(sizeof(unitnos_q));
    p->process = process;
    return p;
  }
}
void unitnos_q_destroy(unitnos_q *q) {
  unitnos_procotol_send_command1(q->process, UNITNOS_Q_COMMAND_CLOSE);
  unitnos_process_close(q->process);
  free(q);
}
pid_t unitnos_q_get_pid(unitnos_q *q) {
  return unitnos_process_get_pid(q->process);
}
void unitnos_q_set_ith(unitnos_q *q, unsigned int ith) {
  unitnos_procotol_send_command_with_data1(
      q->process, UNITNOS_Q_COMMAND_SET_ITH, "%u", ith);
}
void unitnos_q_set_siblings_cnt(unitnos_q *q, unsigned int m) {
  unitnos_procotol_send_command_with_data1(
      q->process, UNITNOS_Q_COMMAND_SET_SIBLINGS_CNT, "%u", m);
}
void unitnos_q_add_new_file(unitnos_q *q, const char *file) {
  unitnos_procotol_send_command_with_data1(
      q->process, UNITNOS_Q_COMMAND_ADD_NEW_FILE, "%s", file);
}
void unitnos_q_remove_file(unitnos_q *q, const char *file) {
  unitnos_procotol_send_command_with_data1(
      q->process, UNITNOS_Q_COMMAND_REMOVE_FILE, "%s", file);
}

void unitnos_q_process(unitnos_q *q, struct unitnos_q_event_callbacks cbs,
                       void *user_data) {
  int in_pipe = unitnos_process_get_fd(q->process, "r");
  if (unitnos_set_non_blocking(in_pipe) == -1) {
    log_error("Failed communication with child");
    return;
  }

  char *message = NULL;
  size_t message_buf_size = 0;
  ssize_t message_len = 0;

  while ((message_len = unitnos_getline(&message, &message_buf_size, in_pipe)) >
         0) {
    struct unitnos_protocol_command command = unitnos_protocol_parse(message);

    log_verbose("Received from child: %s", command.command);
    log_verbose("Received from child value: %s", command.value);

    if (!strcmp(command.command, UNITNOS_Q_SELF_COMMAND_SEND_STATISTICS_FILE)) {
      char file[strlen(command.value) + 1];
      strcpy(file, command.value);

      struct unitnos_char_count_statistics stat;
      int ret = unitnos_char_count_statistics_read(
          UNITNOS_Q_SELF_COMMAND_SEND_STATISTICS_CONTENT, &stat, in_pipe);
      if (ret == 0) {
        cbs.on_new_statistics(q, file, &stat, user_data);
      }
    }
  }

  if (message_len == 0) {
    log_error("Child input pipe closed. Unexpected child termination");
  }

  if (errno == EAGAIN) {
    log_debug("No message from child");
  } else {
    log_error("Unable to read from child: %s", strerror(errno));
  }

  free(message);
}
