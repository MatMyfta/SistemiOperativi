#include "analyzer.h"

#define LOG_TAG "analyzer_parent"
#include "../logger.h"
#include "../protocol.h"
#include "../utils.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct unitnos_analyzer {
  unitnos_process *process;
  FILE *fin;
};

unitnos_analyzer *unitnos_analyzer_create(void) {
  char *const argv[] = {(char *)NULL};
  unitnos_process *process =
      unitnos_program_open(UNITNOS_PROGRAM_ANALYZER, argv);
  if (!process) {
    return NULL;
  } else {
    unitnos_analyzer *analyzer = malloc(sizeof(unitnos_analyzer));
    analyzer->process = process;
    analyzer->fin = fdopen(unitnos_process_get_fd(process, "r"), "r");
    return analyzer;
  }
}
void unitnos_analyzer_delete(unitnos_analyzer *analyzer) {
  unitnos_process_close(analyzer->process);
  free(analyzer);
}

void unitnos_analyzer_set_n(unitnos_analyzer *analyzer, unsigned int n) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(analyzer->process, "w"),
                                 "set_n", "%u", n);
}
void unitnos_analyzer_set_m(unitnos_analyzer *analyzer, unsigned int m) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(analyzer->process, "w"),
                                 "set_m", "%u", m);
}
void unitnos_analyzer_add_new_path(unitnos_analyzer *analyzer,
                                   const char *path) {
  unitnos_procotol_send_command1(unitnos_process_get_fd(analyzer->process, "w"),
                                 "add_new_path", "%s", path);
}

void unitnos_analyzer_process(unitnos_analyzer *analyzer) {
  char *message = NULL;
  size_t message_size = 0;
  while (getline(&message, &message_size, analyzer->fin) > 0) {
    log_verbose("Received: %s", message);
  }
  free(message);
}
