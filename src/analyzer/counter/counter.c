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
  unitnos_procotol_send_command1(counter->process,
                                 UNITNOS_COUNTER_COMMAND_CLOSE);
  unitnos_process_close(counter->process);
  free(counter);
}

void unitnos_counter_set_n(unitnos_counter *counter, unsigned int n) {
  unitnos_procotol_send_command_with_data1(
      counter->process, UNITNOS_COUNTER_COMMAND_SET_N, "%u", n);
}
void unitnos_counter_set_m(unitnos_counter *counter, unsigned int m) {
  unitnos_procotol_send_command_with_data1(
      counter->process, UNITNOS_COUNTER_COMMAND_SET_M, "%u", m);
}

struct add_new_file_batch_context {
  unitnos_counter *counter;
  size_t index;
  size_t size;
};
static bool send_file(void *value, void *user_data) {
  struct add_new_file_batch_context *context =
      (struct add_new_file_batch_context *)user_data;
  const char *file = value;

  if (context->index == context->size - 1) {
    unitnos_procotol_send_command_with_data1(
        context->counter->process,
        UNITNOS_COUNTER_COMMAND_ADD_NEW_FILE_BATCH_FINISH, "%s", file);
  } else {
    unitnos_procotol_send_command_with_data1(
        context->counter->process, UNITNOS_COUNTER_COMMAND_ADD_NEW_FILE_BATCH,
        "%s", file);
  }
  ++context->index;
  return false;
}
void unitnos_counter_add_new_files_batch(unitnos_counter *counter,
                                         unitnos_set *files) {
  struct add_new_file_batch_context context;
  context.size = unitnos_set_size(files);
  context.index = 0;
  context.counter = counter;
  assert(context.size > 0);
  unitnos_set_foreach(files, send_file, &context);
}

void unitnos_counter_process(unitnos_counter *counter) {
  char *message = NULL;
  size_t message_size = 0;
  while (getline(&message, &message_size, counter->fin) > 0) {
    log_verbose("Received: %s", message);
  }
  free(message);
}
