#ifndef UNITNOS_P_H_
#define UNITNOS_P_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UNITNOS_P_COMMAND_SET_M "set_m"
#define UNITNOS_P_COMMAND_ADD_NEW_FILE "add_new_file"

/*******************************************************************************
 * API for parent process
 *******************************************************************************/
typedef struct unitnos_p unitnos_p;
unitnos_p *unitnos_p_create(void);
void unitnos_p_set_m(unitnos_p *p, unsigned int m);
void unitnos_p_add_new_file(unitnos_p *p, const char *file);
void unitnos_p_destroy(unitnos_p *p);

/*******************************************************************************
 * API for p process
 *******************************************************************************/
int unitnos_p_self_main(int in_pipe, int output_pipe);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_P_H_ */
