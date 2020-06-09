#ifndef UNITNOS_Q_H_
#define UNITNOS_Q_H_

#include "../../statistics.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UNITNOS_Q_COMMAND_SET_SIBLINGS_CNT "set_siblings_cnt"
#define UNITNOS_Q_COMMAND_SET_ITH "set_ith"
#define UNITNOS_Q_COMMAND_ADD_NEW_FILE "add_new_file"
#define UNITNOS_Q_COMMAND_REMOVE_FILE "remove_file"
#define UNITNOS_Q_COMMAND_CLOSE "close"

#define UNITNOS_Q_SELF_COMMAND_SEND_STATISTICS_FILE "send_statistics_file"
#define UNITNOS_Q_SELF_COMMAND_SEND_STATISTICS_CONTENT "send_statistics_content"

#include <stdlib.h>

/*******************************************************************************
 * API for parent process
 *******************************************************************************/
typedef struct unitnos_q unitnos_q;
struct unitnos_q_event_callbacks {
  void (*on_new_statistics)(unitnos_q *q, const char *file,
                            struct unitnos_char_count_statistics *statistics,
                            void *user_data);
};
unitnos_q *unitnos_q_create(void);
void unitnos_q_destroy(unitnos_q *q);
pid_t unitnos_q_get_pid(unitnos_q *q);
void unitnos_q_set_ith(unitnos_q *q, unsigned int ith);
void unitnos_q_set_siblings_cnt(unitnos_q *q, unsigned int m);
void unitnos_q_add_new_file(unitnos_q *q, const char *file);
void unitnos_q_remove_file(unitnos_q *q, const char *file);
void unitnos_q_process(unitnos_q *q, struct unitnos_q_event_callbacks cbs,
                       void *user_data);

/*******************************************************************************
 * API for q process
 *******************************************************************************/
int unitnos_q_self_main(int in_pipe, int output_pipe);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_Q_H_ */
