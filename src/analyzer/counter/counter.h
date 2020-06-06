#ifndef UNITNOS_COUNTER_H_
#define UNITNOS_COUNTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UNITNOS_COUNTER_COMMAND_SET_N "set_n"
#define UNITNOS_COUNTER_COMMAND_SET_M "set_m"
#define UNITNOS_COUNTER_COMMAND_ADD_NEW_PATH "add_new_path"
#define UNITNOS_COUNTER_COMMAND_LIST_PATHS "list_paths"

/*******************************************************************************
 * API for parent process
 *******************************************************************************/
typedef struct unitnos_counter unitnos_counter;
unitnos_counter *unitnos_counter_create(void);
void unitnos_counter_set_n(unitnos_counter *counter, unsigned int n);
void unitnos_counter_set_m(unitnos_counter *counter, unsigned int m);
void unitnos_counter_add_new_path(unitnos_counter *counter, const char *path);
void unitnos_counter_list_paths(unitnos_counter *counter);
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
