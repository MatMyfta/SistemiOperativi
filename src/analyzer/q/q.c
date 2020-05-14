#include "q.h"

#include "../../process.h"

#include <unistd.h>
#include <stdlib.h>

struct unitnos_q {
  unitnos_process *process;
};

unitnos_q *unitnos_q_create() {
  unitnos_q *q = malloc(sizeof(unitnos_q));
  char *const argv[] = {(char *)NULL};
  unitnos_process_open("q", argv);
  return q;

}
void unitnos_q_update(unitnos_q *q, uint16_t all_children_cnt, uint16_t ith,
                      uint16_t filec, char *filev[]) {}
void unitnos_q_destroy(unitnos_q *q) {
  unitnos_process_close(q->process);
  free(q);
}
