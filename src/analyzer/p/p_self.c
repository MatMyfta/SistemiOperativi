#include "../q/q.h"
#include "p.h"

#define LOG_TAG "p"
#include "../../logger.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>

int unitnos_p_self_main(int in_pipe, int output_pipe) {
  log_debug("Started");

  char outbuf[64];
  snprintf(outbuf, 64, "Hello World from %d", getpid());
  write(output_pipe, outbuf, strlen(outbuf));

  char buf[30] = {};
  while (read(in_pipe, buf, 30) > 0) {
    log_debug("Received %s from parent", buf);
    break;
  }

  unitnos_q *q_children[3];

  int i;
  for (i = 0; i < 3; ++i) {
    q_children[i] = unitnos_q_create();
  }

  // MANCANO TUTTE LE FUNZIONI DI Q, L'INIZIALIZZAZIONE E' UNA PROVA

  // Aggiunti gli include mancanti

  return 0;
}
