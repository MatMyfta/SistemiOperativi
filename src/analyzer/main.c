#include "analyzer.h"

#include "counter/counter.h"

#define LOG_TAG "analyzer"
#include "../logger.h"

#include "../process.h"
#include "../protocol.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int independent_analyzer_main(int argc, char **argv);
static int child_analyzer_main(int in_pipe, int output_pipe);

int main(int argc, char *argv[]) {
  if (unitnos_process_is_process(argc, argv)) {
    return child_analyzer_main(atoi(argv[1]), atoi(argv[2]));
  } else {
    return independent_analyzer_main(argc, argv);
  }
}

static int independent_analyzer_main(int argc, char **argv) {
  log_debug("Analyzer running in standalone mode");
  if (argc < 4) {
    log_error("Usage: analyzer N M PATH...");
    return -1;
  }

  unitnos_counter *counter = unitnos_counter_create();
  if (!counter) {
    log_error("Unable to create counter");
    return -1;
  }

  unitnos_counter_set_n(counter, atoi(argv[1]));
  unitnos_counter_set_m(counter, atoi(argv[2]));
  {
    int i;
    for (i = 3; i < argc; ++i) {
      unitnos_counter_add_new_path(counter, argv[i]);
    }
  }
  while (1) {
    unitnos_counter_process(counter);
  }
  unitnos_counter_delete(counter);
  return 0;
}

static int child_analyzer_main(int in_pipe, int output_pipe) {
  log_debug("Analyzer running in child mode");

  FILE *fin = fdopen(in_pipe, "r");

  unitnos_counter *counter = unitnos_counter_create();
  if (!counter) {
    log_error("Unable to create counter");
    return -1;
  }

  char *message = NULL;
  size_t message_size = 0;

  while (1) {
    if (getline(&message, &message_size, fin) >= 0) {
      struct unitnos_protocol_command command = unitnos_protocol_parse(message);
      log_verbose("Received command: %s", command.command);

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_SET_N)) {
        unsigned int n;
        int ret = sscanf(command.value, "%u", &n);
        assert(ret > 0);
        unitnos_counter_set_n(counter, n);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_SET_M)) {
        unsigned int m;
        int ret = sscanf(command.value, "%u", &m);
        assert(ret > 0);
        unitnos_counter_set_m(counter, m);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_ADD_NEW_PATH)) {
        unitnos_counter_add_new_path(counter, command.value);
      }

      if (!strcmp(command.command, UNITNOS_ANALYZER_COMMAND_LIST_PATHS)) {
        unitnos_counter_list_paths(counter);
      }
    } else if (feof(fin)) {
      log_debug("Input pipe closed. Terminate");
      unitnos_counter_delete(counter);
      break;
    }
  }

  return 0;
}
