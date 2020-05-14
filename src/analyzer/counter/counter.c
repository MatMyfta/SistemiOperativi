#include "counter.h"

#define LOG_TAG "counter_parent"
#include "../../logger.h"
#include "../../utils.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct unitnos_counter {
  unitnos_process *process;
};

unitnos_counter *unitnos_counter_create(void) {
  char *const argv[] = {(char *)NULL};
  unitnos_process *process =
      unitnos_program_open(UNITNOS_PROGRAM_COUNTER, argv);
  if (!process) {
    return NULL;
  } else {
    unitnos_counter *counter = malloc(sizeof(unitnos_counter));
    counter->process = process;
    return counter;
  }
}
void unitnos_counter_delete(unitnos_counter *counter) {
  unitnos_process_close(counter->process);
  free(counter);
}
void unitnos_counter_set(unitnos_counter *counter, uint16_t n, uint16_t m,
                         char *filev[]) {
  int fd = unitnos_process_get_fd(counter->process, "w");
  char buf[] = "Hello World";
  write(fd, buf, strlen(buf));
}

void unitnos_counter_read(unitnos_counter *counter) {
  int fd = unitnos_process_get_fd(counter->process, "r");
  char buf[30];
  int received = read(fd, buf, 30);
  log_debug("Received %s", buf);
}
