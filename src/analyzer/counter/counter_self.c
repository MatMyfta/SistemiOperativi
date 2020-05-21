#include "../p/p.h"
#include "counter.h"

#define LOG_TAG "counter"
#include "../../logger.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>

int unitnos_counter_self_main(int in_pipe, int output_pipe) {
  log_debug("Started");

  char outbuf[64];
  snprintf(outbuf, 64, "Hello World from %d", getpid());
  write(output_pipe, outbuf, strlen(outbuf));

  char buf[30] = {};
  while (read(in_pipe, buf, 30) > 0) {
    log_debug("Received %s from parent", buf);
  }

  unitnos_p *p_children[5];

  int i;
  for (i = 0; i < 5; ++i) {
    p_children[i] = unitnos_p_create();
  }

  for (i = 0; i < 5; ++i) {
    unitnos_p_set(p_children[i], 5, NULL);
  }

  for (i = 0; i < 5; ++i) {
    unitnos_p_read(p_children[i]);
  }

  for (i = 0; i < 5; ++i) {
    unitnos_p_destroy(p_children[i]);
  }
  return 0;
}
