#include "path.h"

#define LOG_TAG "analyzer"
#include "../logger.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIND_COMMNAD_PRE "find "
/**
 * We need to redirect stderr to stdout, because popen executes the passed
 * command through a shell, i.e. `sh -c <command>`.
 * When the shell can't execute the command, it prints the error message to
 * stderr. With popen we can read only stdout.
 */
#define FIND_COMMNAD_POST " -type f 2>&1"

unitnos_set *unitnos_unpack_path(const char *path) {
  /*
   * In docker il comando find si trova in /usr/bin/find invece che in /bin/find
   * Comportamenti analoghi e controllati per entrambi gli ambienti.
   *
   */
  size_t command_len =
      strlen(path) + sizeof(FIND_COMMNAD_PRE) + sizeof(FIND_COMMNAD_POST);
  char command[command_len];
  int ret = snprintf(command, command_len,
                     FIND_COMMNAD_PRE "%s" FIND_COMMNAD_POST, path);
  assert(ret > 0 && (size_t)ret <= command_len);

  log_verbose("Unpacking path with command %s", command);

  FILE *fp = popen(command, "r");

  if (!fp) {
    log_error("Unable to parse path with /bin/find %s", strerror(errno));
    return NULL;
  }

  unitnos_set *set =
      unitnos_set_create(unitnos_container_util_strcmp, NULL, NULL);

  char *file_path_buf = NULL;
  size_t file_path_buf_size = 0;
  ssize_t file_path_size = 0;

  struct stat statbuf;
  while ((file_path_size = getline(&file_path_buf, &file_path_buf_size, fp)) >
         0) {
    // discard '\n'
    file_path_buf[file_path_size - 1] = '\0';

    if (stat(file_path_buf, &statbuf) == -1) {
      log_error("%s", file_path_buf);
      // we can reuse the allocated file_path_buf
      continue;
    }

    // we use find -type f, which returns only regular files
    assert(S_ISREG(statbuf.st_mode));
    if (file_path_buf_size > PATH_MAX) {
      log_warn("A file path longer than %d (PATH_MAX) has been received, "
               "it'll be ignored",
               PATH_MAX);
      log_warn("Path in question: %s", file_path_buf);
      // we can reuse the allocated file_path_buf
      continue;
    }

    unitnos_set_insert(set, file_path_buf);
    // set to NULL, so that getline reallocates another string for the next path
    file_path_buf = NULL;
    file_path_buf_size = 0;
  }
  pclose(fp);

  if (unitnos_set_size(set) == 0) {
    unitnos_set_destroy(set);
    return NULL;
  }

  return set;
}
