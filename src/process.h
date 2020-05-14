/**
 * \file process.h
 *
 * \brief Process managment related utils - interface
 */
#ifndef UNITNOS_PROCESS_H_
#define UNITNOS_PROCESS_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct unitnos_process unitnos_process;


/**
 * Opens a process by creating two pipes, forking, and invoking execv with
 * provided arguments.
 *
 * \param [in] path passed to execv
 * \param [in] argv passed to execv
 *
 * This function invokes execv by passing as argv[1] the input pipe and as
 * argv[2] the output pipe. argv[0] is left as is, whereas the other potential
 * arguments in the given \p argv are shifted by two places.
 */
unitnos_process *unitnos_process_open(const char *path, char *const *argv);
int unitnos_process_close(unitnos_process *p);
int unitnos_process_get_fd(unitnos_process *p, const char *mode);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_PROCESS_H_ */
