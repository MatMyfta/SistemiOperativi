#ifndef UNITNOS_Q_H_
#define UNITNOS_Q_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UNITNOS_Q_COMMAND_SET_SIBLINGS_CNT "set_siblings_cnt"
#define UNITNOS_Q_COMMAND_SET_ITH "set_ith"
#define UNITNOS_Q_COMMAND_ADD_NEW_FILE "add_new_file"

/*******************************************************************************
 * API for parent process
 *******************************************************************************/
typedef struct unitnos_q unitnos_q;
unitnos_q *unitnos_q_create(void);
void unitnos_q_destroy(unitnos_q *q);
void unitnos_q_set_ith(unitnos_q *q, unsigned int ith);
void unitnos_q_set_siblings_cnt(unitnos_q *q, unsigned int m);
void unitnos_q_add_new_file(unitnos_q *q, const char *file);

/*******************************************************************************
 * API for q process
 *******************************************************************************/
int unitnos_q_self_main(int in_pipe, int output_pipe);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_Q_H_ */
