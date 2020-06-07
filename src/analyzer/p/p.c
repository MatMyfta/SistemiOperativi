#include "p.h"

#include "../../process.h"
#include "../../utils.h"
#define LOG_TAG "p_parent"
#include "../../logger.h"
#include "../../protocol.h"

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
    unitnos_p *p = malloc(sizeof(unitnos_p));
    p->process = process;
    return p;
  }
}
void unitnos_p_destroy(unitnos_p *p) {
  unitnos_process_close(p->process);
  free(p);
}

void unitnos_p_set_m(unitnos_p *p, unsigned int m) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(p->process, "w"),
                                 UNITNOS_P_COMMAND_SET_M, "%u", m);
}
void unitnos_p_add_new_file(unitnos_p *p, const char *file) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(p->process, "w"),
                                 UNITNOS_P_COMMAND_ADD_NEW_FILE, "%s", file);
}
void unitnos_p_read(unitnos_p *p) {
  int fd = unitnos_process_get_fd(p->process, "r");
  char buf[30];
  int received = read(fd, buf, 30);
  log_debug("Received %s", buf);
}
