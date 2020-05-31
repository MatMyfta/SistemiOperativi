#include "q.h"

#include "../../process.h"
#include "../../protocol.h"
#include "../../utils.h"

#define LOG_TAG "q_qarent"
#include "../../logger.h"

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
void unitnos_q_destroy(unitnos_q *p) {
  unitnos_process_close(p->process);
  free(p);
}

void unitnos_q_set_ith(unitnos_q *q, unsigned int ith) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(q->process, "w"),
                                 UNITNOS_Q_COMMAND_SET_ITH, "%u", ith);
}
void unitnos_q_set_siblings_cnt(unitnos_q *q, unsigned int m) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(q->process, "w"),
                                 UNITNOS_Q_COMMAND_SET_SIBLINGS_CNT, "%u", m);
}
void unitnos_q_add_new_file(unitnos_q *q, const char *file) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(q->process, "w"),
                                 UNITNOS_Q_COMMAND_SET_ITH, "%s", file);
}

void unitnos_q_read(unitnos_q *p) {
  int fd = unitnos_process_get_fd(p->process, "r");
  char buf[30];
  int received = read(fd, buf, 30);
  log_debug("Received %s", buf);
}
