/**
 * \file protocol.h
 *
 * \brief Basic protocol for message exchange over pipe - interface
 */

#ifndef UNITNOS_PROTOCOL_H_
#define UNITNOS_PROTOCOL_H_

#include "process.h"

#include <signal.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UNITNOS_PROTOCOL_COMMAND_VALUE_DELIMITATOR ':'

/**
 * Setup SIGUSR1
 *
 * \retval 0 success
 * \retval -1 failure
 */
int unitnos_procotol_init();
int unitnos_procotol_deinit();

void unitnos_procotol_send_command(int fd, pid_t pid, const char *command);
void unitnos_procotol_send_command1(unitnos_process *process,
                                    const char *command);

#define unitnos_procotol_send_command_with_data(fd, pid, command, fmt,         \
                                                args...)                       \
  do {                                                                         \
    size_t command_len = strlen(command);                                      \
    char buf[2 /* for ':' separator and newline */                             \
             + command_len + snprintf(NULL, 0, fmt, args)];                    \
    strcpy(buf, command);                                                      \
    buf[command_len] = ':';                                                    \
    sprintf(buf + command_len + 1, fmt, args);                                 \
    buf[sizeof(buf) - 1] = '\n';                                               \
    write(fd, buf, sizeof buf);                                                \
    kill(pid, SIGUSR1);                                                        \
  } while (0)
#define unitnos_procotol_send_command_with_data1(process, command, fmt,        \
                                                 args...)                      \
  do {                                                                         \
    unitnos_procotol_send_command_with_data(                                   \
        unitnos_process_get_fd(process, "w"),                                  \
        unitnos_process_get_pid(process), command, fmt, args);                 \
  } while (0)

/**
 * `snprintf()` can't be used to encode binary data
 *
 * Apparently %.*s stops at the first null terminator, instead of printing all
 * the requested bytes.
 */
void unitnos_procotol_send_command_with_binary_data(int fd, pid_t pid,
                                                    const char *command,
                                                    void *data, size_t size);
void unitnos_procotol_send_command_with_binary_data1(unitnos_process *process,
                                                     const char *command,
                                                     void *data, size_t size);

struct unitnos_protocol_command {
  /**
   * The name of the command terminated with a null character
   */
  const char *command;
  /**
   * The portion of a command after the `:` separator terminated with a null
   * character.
   *
   * NULL if not present.
   */
  const char *value;
};

/**
 * Utility function that splits a command into command name and value.
 * The newline delimiter is stripped.
 *
 * \param [in] message a stringa terminated with a '\n'
 *
 * Note that with function modifies the given message and returns pointers to
 * it. Pay attention when dealing with memory management.
 */
struct unitnos_protocol_command unitnos_protocol_parse(char *message);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_PROTOCOL_H_ */
