#include "p.h"

#include "../../process.h"
#include "../../utils.h"
#define LOG_TAG "p_parent"
#include "../../logger.h"
#include "../../protocol.h"
#include "../../statistics.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct unitnos_p {
  unitnos_process *process;
};

unitnos_p *unitnos_p_create(void) {
  char *const argv[] = {(char *)NULL};
  unitnos_process *process = unitnos_program_open(UNITNOS_PROGRAM_P, argv);
  if (!process) {
    return NULL;
  } else {
    unitnos_p *p = unitnos_malloc(sizeof(unitnos_p));
    p->process = process;
    return p;
  }
}
void unitnos_p_destroy(unitnos_p *p) {
  unitnos_procotol_send_command1(p->process, UNITNOS_P_COMMAND_CLOSE);
  unitnos_process_close(p->process);
  free(p);
}
pid_t unitnos_p_get_pid(unitnos_p *p) {
  return unitnos_process_get_pid(p->process);
}
void unitnos_p_status(unitnos_p *p) {
  unitnos_procotol_send_command1(p->process, UNITNOS_P_COMMAND_STATUS);
}
void unitnos_p_set_m(unitnos_p *p, unsigned int m) {
  unitnos_procotol_send_command_with_data1(p->process, UNITNOS_P_COMMAND_SET_M,
                                           "%u", m);
}
void unitnos_p_add_new_file(unitnos_p *p, const char *file) {
  unitnos_procotol_send_command_with_data1(
      p->process, UNITNOS_P_COMMAND_ADD_NEW_FILE, "%s", file);
}
void unitnos_p_remove_file(unitnos_p *p, const char *file) {
  unitnos_procotol_send_command_with_data1(
      p->process, UNITNOS_P_COMMAND_REMOVE_FILE, "%s", file);
}
void unitnos_p_read(unitnos_p *p) {
  int fd = unitnos_process_get_fd(p->process, "r");
  char buf[30];
  int received = read(fd, buf, 30);
  log_debug("Received %s", buf);
}

void unitnos_p_process(unitnos_p *p, struct unitnos_p_event_callbacks cbs,
                       void *user_data) {
  int in_pipe = unitnos_process_get_fd(p->process, "r");
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

    if (!strcmp(command.command, UNITNOS_P_SELF_COMMAND_SEND_STATISTICS_FILE)) {
      char file[strlen(command.value) + 1];
      strcpy(file, command.value);

      struct unitnos_char_count_statistics stat;
      int ret = unitnos_char_count_statistics_read(
          UNITNOS_P_SELF_COMMAND_SEND_STATISTICS_CONTENT, &stat, in_pipe);
      if (ret == 0) {
        cbs.on_new_statistics(p, file, &stat, user_data);
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
