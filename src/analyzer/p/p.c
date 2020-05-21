#include "p.h"

#include "../../process.h"
#include "../../utils.h"
#define LOG_TAG "p_parent"
#include "../../logger.h"

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
    unitnos_p *p = malloc(sizeof(unitnos_p)); // Lijun: tu riportavi "p" e ho pensato fosse una svista
    p->process = process;
    return p;
  }
}
void unitnos_p_destroy(unitnos_p *p) {
  unitnos_process_close(p->process);
  free(p);
}

void unitnos_p_set(unitnos_p *p, uint16_t m, char *filev[]) {
  int fd = unitnos_process_get_fd(p->process, "w");
  char buf[] = "Hello World";
  write(fd, buf, strlen(buf));
}
void unitnos_p_read(unitnos_p *p) {
  int fd = unitnos_process_get_fd(p->process, "r");
  char buf[30];
  int received = read(fd, buf, 30);
  log_debug("Received %s", buf);
}
