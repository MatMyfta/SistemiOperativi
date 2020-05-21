#ifndef UNITNOS_COUNTER_H_
#define UNITNOS_COUNTER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API for parent process
 *******************************************************************************/
typedef struct unitnos_counter unitnos_counter;
unitnos_counter *unitnos_counter_create(void);
void unitnos_counter_set(unitnos_counter *counter, uint16_t n, uint16_t m,
                         char *filev[]);
void unitnos_counter_read(unitnos_counter *counter);
void unitnos_counter_delete(unitnos_counter *counter);

/*******************************************************************************
 * API for the counter process
 *******************************************************************************/
int unitnos_counter_self_main(int in_pipe, int output_pipe);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_COUNTER_H_ */
