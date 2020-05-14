#include "counter/counter.h"

#define LOG_TAG "analyzer"
#include "../logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 4) {
    log_error("Usage: ./analyzer <n> <m> <file-list>");
    return -1;
  }

  log_debug("Started");
  unitnos_counter *counter = unitnos_counter_create();
  if (!counter) {
    log_error("Unable to create counter");
    return -1;
  }
  unitnos_counter_set(counter, atoi(argv[1]), atoi(argv[2]), argv + 3);
  unitnos_counter_read(counter);
  unitnos_counter_delete(counter);
  return 0;
}
