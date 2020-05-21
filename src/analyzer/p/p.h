#ifndef UNITNOS_P_H_
#define UNITNOS_P_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API for parent process
 *******************************************************************************/
typedef struct unitnos_p unitnos_p;
unitnos_p *unitnos_p_create(void);
void unitnos_p_set(unitnos_p *p, uint16_t m, char *filev[]);
void unitnos_p_read(unitnos_p *p);
void unitnos_p_destroy(unitnos_p *p);

/*******************************************************************************
 * API for p process
 *******************************************************************************/
int unitnos_p_self_main(int in_pipe, int output_pipe);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_P_H_ */
