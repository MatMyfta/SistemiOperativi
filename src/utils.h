/**
 * \file utils.h
 *
 * \brief Common utilities - interface
 */

#ifndef UNITNOS_UTILS_H_
#define UNITNOS_UTILS_H_

#include "process.h"

#ifdef __cplusplus
extern "C" {
#endif

enum unitnos_program {
  UNITNOS_PROGRAM_MAIN,
  UNITNOS_PROGRAM_ANALYZER,
  UNITNOS_PROGRAM_COUNTER,
  UNITNOS_PROGRAM_P,
  UNITNOS_PROGRAM_Q,
  UNITNOS_PROGRAM_REPORT
};

/**
 * Wrapper of #unitnos_program_open.
 *
 * It assumes that all the programs are found under the same directory.
 *
 * \param [in] the program you want to open
 * \param [in] argv arguments to be passed
 */
unitnos_process *unitnos_program_open(enum unitnos_program program,
                                      char *const *argv);

/**
 * Given an canonical argv, it returns the corresponding argc
 */
int unitnos_get_argc(char *const *argv);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_UTILS_H_ */
