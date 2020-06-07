#include "counter.h"

#define LOG_TAG "counter_parent"
#include "../../logger.h"
#include "../../protocol.h"
#include "../../utils.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct unitnos_counter {
  unitnos_process *process;
  FILE *fin;
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
    counter->fin = fdopen(unitnos_process_get_fd(process, "r"), "r");
    return counter;
  }
}
void unitnos_counter_delete(unitnos_counter *counter) {
  unitnos_process_close(counter->process);
  free(counter);
}

void unitnos_counter_set_n(unitnos_counter *counter, unsigned int n) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(counter->process, "w"),
                                 UNITNOS_COUNTER_COMMAND_SET_N, "%u", n);
}
void unitnos_counter_set_m(unitnos_counter *counter, unsigned int m) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(counter->process, "w"),
                                 UNITNOS_COUNTER_COMMAND_SET_M, "%u", m);
}
void unitnos_counter_add_new_file(unitnos_counter *counter, const char *path) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(counter->process, "w"),
                                 UNITNOS_COUNTER_COMMAND_ADD_NEW_FILE, "%s",
                                 path);
}

void unitnos_counter_process(unitnos_counter *counter) {
  char *message = NULL;
  size_t message_size = 0;
  while (getline(&message, &message_size, counter->fin) > 0) {
    log_verbose("Received: %s", message);
  }
  free(message);
}
