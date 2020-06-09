/**
 * \file process.h
 *
 * \brief Process managment related utils - interface
 */
#ifndef UNITNOS_PROCESS_H_
#define UNITNOS_PROCESS_H_

#include "bool.h"

#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct unitnos_process unitnos_process;

/**
 * Open a process by creating two pipes, forking, and invoking execv with
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
/**
 * A child process should call this function when it starts.
 * This function is used to synchrone the child with the parent.
 * Before calling the function, the child could set up things like signal
 * handler, etc.
 */
void unitnos_process_init(int in_pipe, int out_pipe);
/**
 * Destroy and terminate, if not yet terminated, the given process and return
 * the exit value of the process.
 *
 * \param [in] p handle of the process to be destroyed
 */
int unitnos_process_close(unitnos_process *p);
/**
 * Get file descriptor that can be used to communicated with a created process
 *
 * \param [in] p handle of the created process
 * \param [in] mode "r" for input file descriptor and "w" for output file
 * descriptor
 */
int unitnos_process_get_fd(unitnos_process *p, const char *mode);
pid_t unitnos_process_get_pid(unitnos_process *p);
/**
 * Given the argc and argv passed to the main function of a process, determine
 * whether the process has been created by #unitnos_process_open.
 *
 * \param [in] argc argc received by main functionj
 * \param [in] argv argv received by main functionj
 */
bool unitnos_process_is_process(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_PROCESS_H_ */
