#ifndef UNITNOS_ANALYZER_H_
#define UNITNOS_ANALYZER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UNITNOS_ANALYZER_COMMAND_SET_N "set_n"
#define UNITNOS_ANALYZER_COMMAND_SET_M "set_m"
#define UNITNOS_ANALYZER_COMMAND_ADD_NEW_PATH "add_new_path"
#define UNITNOS_ANALYZER_COMMAND_LIST_PATHS "list_paths"
#define UNITNOS_ANALYZER_COMMAND_CLOSE "close"
#define UNITNOS_ANALYZER_COMMAND_STATUS_PANEL "status_panel"

/*******************************************************************************
 * API for parent process
 *******************************************************************************/
typedef struct unitnos_analyzer unitnos_analyzer;
unitnos_analyzer *unitnos_analyzer_create(void);
void unitnos_analyzer_set_n(unitnos_analyzer *analyzer, unsigned int n);
void unitnos_analyzer_set_m(unitnos_analyzer *analyzer, unsigned int m);
void unitnos_analyzer_add_new_path(unitnos_analyzer *analyzer,
                                   const char *path);
void unitnos_analyzer_list_paths(unitnos_analyzer *analyzer);
void unitnos_analyzer_status_panel(unitnos_analyzer *analyzer);
/**
 * Process any unread messages in the pipe
 */
void unitnos_analyzer_process(unitnos_analyzer *analyzer);
void unitnos_analyzer_delete(unitnos_analyzer *analyzer);

/*******************************************************************************
 * API for the analyzer process
 *******************************************************************************/
int unitnos_analyzer_self_main(int in_pipe, int output_pipe);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_ANALYZER_H_ */
