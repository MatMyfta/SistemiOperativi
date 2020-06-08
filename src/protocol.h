/**
 * \file protocol.h
 *
 * \brief Basic protocol for message exchange over pipe - interface
 */

#ifndef UNITNOS_PROTOCOL_H_
#define UNITNOS_PROTOCOL_H_

#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define unitnos_procotol_send_command(fd, command)                             \
  do {                                                                         \
    char buf[strlen(command) + 1];                                             \
    strcpy(buf, command);                                                      \
    buf[sizeof(buf) - 1] = '\n';                                               \
    write(fd, buf, sizeof(buf));                                               \
  } while (0)

#define unitnos_procotol_send_command1(fd, command, fmt, args...)              \
  do {                                                                         \
    size_t command_len = strlen(command);                                      \
    char buf[2 /* for ':' separator and newline */                             \
             + command_len + snprintf(NULL, 0, fmt, args)];                    \
    strcpy(buf, command);                                                      \
    buf[command_len] = ':';                                                    \
    sprintf(buf + command_len + 1, fmt, args);                                 \
    buf[sizeof(buf) - 1] = '\n';                                               \
    write(fd, buf, sizeof buf);                                                \
  } while (0)

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
