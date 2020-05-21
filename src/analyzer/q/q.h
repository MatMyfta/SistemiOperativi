#ifndef UNITNOS_Q_H_
#define UNITNOS_Q_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API for parent process
 *******************************************************************************/
typedef struct unitnos_q unitnos_q;
unitnos_q *unitnos_q_create(void);
void unitnos_q_update(unitnos_q *q, uint16_t all_children_cnt, uint16_t ith,
                      uint16_t filec, char *filev[]);
void unitnos_q_destroy(unitnos_q *q);

/*******************************************************************************
 * API for q process
 *******************************************************************************/
int unitnos_q_self_main(int in_pipe, int output_pipe);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_Q_H_ */
