/**
 * \file utils.c
 *
 * \brief Common utilities - implementation
 */

#include "utils.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int unitnos_get_argc(char *const *argv) {
  if (argv == NULL) {
    return 0;
  }
  int i = 0;
  while (argv[i] != NULL) {
    ++i;
  }
  return i;
}

/**
 * See https://stackoverflow.com/q/4934947/9894313
 */
unitnos_process *unitnos_program_open(enum unitnos_program program,
                                      char *const *argv) {
  static const char *const executable_names[] = {
      [UNITNOS_PROGRAM_MAIN] = "main",
      [UNITNOS_PROGRAM_ANALYZER] = "analyzer",
      [UNITNOS_PROGRAM_COUNTER] = "counter",
      [UNITNOS_PROGRAM_P] = "p",
      [UNITNOS_PROGRAM_Q] = "q",
      [UNITNOS_PROGRAM_REPORT] = "report"};

  char current_exec_path[PATH_MAX];
  // readlink does not null terminate!
  memset(current_exec_path, 0, sizeof(current_exec_path));
  // get the path of the executable of the current process
  readlink("/proc/self/exe", current_exec_path, PATH_MAX);
  // find the last '/' in path
  char *pos = strrchr(current_exec_path, '/');
  // stop string after slash
  *(pos + 1) = '\0';

  char program_path[PATH_MAX];
  snprintf(program_path, PATH_MAX, "%s%s", current_exec_path,
           executable_names[program]);

  size_t argc = unitnos_get_argc(argv);
  const char *new_argv[argc + 2];
  new_argv[0] = executable_names[program];
  {
    size_t i;
    for (i = 1; i < argc; ++i) {
      new_argv[i] = argv[i - 1];
    }
    new_argv[i] = (char *)NULL;
  }
  return unitnos_process_open(program_path, (char **)new_argv);
}
