#ifndef UNITNOS_P_H_
#define UNITNOS_P_H_

#include "../../statistics.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UNITNOS_P_COMMAND_SET_M "set_m"
#define UNITNOS_P_COMMAND_ADD_NEW_FILE "add_new_file"
#define UNITNOS_P_COMMAND_REMOVE_FILE "remove_file"
#define UNITNOS_P_COMMAND_STATUS "status"
#define UNITNOS_P_COMMAND_CLOSE "close"

#define UNITNOS_P_SELF_COMMAND_SEND_STATISTICS_FILE "send_statistics_file"
#define UNITNOS_P_SELF_COMMAND_SEND_STATISTICS_CONTENT "send_statistics_content"
#include <stdlib.h>

/*******************************************************************************
 * API for parent process
 *******************************************************************************/
typedef struct unitnos_p unitnos_p;
unitnos_p *unitnos_p_create(void);
void unitnos_p_set_m(unitnos_p *p, unsigned int m);
void unitnos_p_add_new_file(unitnos_p *p, const char *file);
void unitnos_p_remove_file(unitnos_p *p, const char *file);
void unitnos_p_destroy(unitnos_p *p);
pid_t unitnos_p_get_pid(unitnos_p *p);
void unitnos_p_status(unitnos_p *p);

struct unitnos_p_event_callbacks {
  void (*on_new_statistics)(unitnos_p *p, const char *file,
                            struct unitnos_char_count_statistics *statistics,
                            void *user_data);
};
void unitnos_p_process(unitnos_p *p, struct unitnos_p_event_callbacks cbs,
                       void *user_data);

/*******************************************************************************
 * API for p process
 *******************************************************************************/
int unitnos_p_self_main(int in_pipe, int output_pipe);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_P_H_ */
