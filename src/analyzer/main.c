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

  int n = atoi(argv[1]);
  int m = atoi(argv[2]);

  unitnos_analyzer *analyzer = unitnos_analyzer_create();

  if (!analyzer) {
    log_error("Unable to create child process");
    return -1;
  }

  unitnos_analyzer_set_n(analyzer, n);
  unitnos_analyzer_set_m(analyzer, m);
  {
    int i;
    for (i = 3; i < argc; ++i) {
      unitnos_analyzer_add_new_path(analyzer, argv[i]);
    }
  }
  unitnos_analyzer_delete(analyzer);
  return 0;
}

static int child_analyzer_main(int in_pipe, int output_pipe) {
  log_debug("Analyzer running in child mode");
  unitnos_analyzer_self_main(in_pipe, output_pipe);

  return 0;
}
