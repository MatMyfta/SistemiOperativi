#ifndef UNITNOS_COUNTER_H_
#define UNITNOS_COUNTER_H_

#include "../../containers/set.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UNITNOS_COUNTER_COMMAND_SET_N "set_n"
#define UNITNOS_COUNTER_COMMAND_SET_M "set_m"
#define UNITNOS_COUNTER_COMMAND_ADD_NEW_FILE_BATCH "add_new_file_batch"
#define UNITNOS_COUNTER_COMMAND_ADD_NEW_FILE_BATCH_FINISH                      \
  "add_new_file_batch_finish"

/*******************************************************************************
 * API for parent process
 *******************************************************************************/
typedef struct unitnos_counter unitnos_counter;
unitnos_counter *unitnos_counter_create(void);
void unitnos_counter_set_n(unitnos_counter *counter, unsigned int n);
void unitnos_counter_set_m(unitnos_counter *counter, unsigned int m);
void unitnos_counter_add_new_files_batch(unitnos_counter *counter,
                                        unitnos_set *files);
/**
 * Process any unread messages in the pipe
 */
void unitnos_counter_process(unitnos_counter *counter);
void unitnos_counter_delete(unitnos_counter *counter);

/*******************************************************************************
 * API for the counter process
 *******************************************************************************/
int unitnos_counter_self_main(int in_pipe, int output_pipe);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_COUNTER_H_ */
