/**
 * \file process.c
 *
 * \brief Process managment related utils - implementation
 *
 * Adapted from APUE
 */

#include "process.h"
#include "utils.h"

#define LOG_TAG "process"
#include "logger.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * See https://stackoverflow.com/a/3437484/9894313
 * Prevent double evaluation
 */
#define MAX(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a > _b ? _a : _b;                                                         \
  })

struct unitnos_process {
  pid_t pid;
  int pipe_in;
  int pipe_out;
};

unitnos_process *unitnos_process_open(const char *path, char *const *argv) {
  pid_t pid;
  /* pipe: parent in - child out */
  int pico[2];
  /* pipe: parent out - child in */
  int poci[2];

  if (pipe(pico) < 0)
    return NULL;
  if (pipe(poci) < 0) {
    close(pico[0]);
    close(pico[1]);
    return NULL;
  }

  if ((pid = fork()) < 0) {
    log_error("Unable to fork: %s", strerror(errno));
    return NULL;
  } else if (pid == 0) {
    close(pico[0]);
    close(poci[1]);

    int argc = unitnos_get_argc(argv);
    const char *new_argv[MAX(argc + 3, 4)];
    new_argv[0] = argc > 0 ? argv[0] : "";
    char inpipe_buf[12];
    char outpipe_buf[12];
    snprintf(inpipe_buf, 12, "%d", poci[0]);
    snprintf(outpipe_buf, 12, "%d", pico[1]);
    new_argv[1] = inpipe_buf;
    new_argv[2] = outpipe_buf;
    {
      int i;
      for (i = 3; i < argc; ++i) {
        new_argv[i] = argv[i - 3];
      }
      new_argv[i] = (char *)NULL;
    }
    execvp(path, (char**)new_argv);
    log_error("Unable to execute child process: %s", strerror(errno));
    _exit(127);
  }

  close(pico[1]);
  close(poci[0]);
  {
    unitnos_process *process = malloc(sizeof(unitnos_process));
    process->pid = pid;
    process->pipe_in = pico[0];
    process->pipe_out = poci[1];
    return process;
  }
}

int unitnos_process_close(unitnos_process *p) {
  int stat = -1;
  if (close(p->pipe_in) == -1) {
    goto exit;
  }
  if (close(p->pipe_out) == -1) {
    goto exit;
  }
  while (waitpid(p->pid, &stat, 0) < 0) {
    if (errno != EINTR)
      goto exit;
  }

exit:
  free(p);
  return stat;
}

int unitnos_process_get_fd(unitnos_process *p, const char *mode) {
  if (*mode == 'r') {
    return p->pipe_in;
  } else {
    return p->pipe_out;
  }
}