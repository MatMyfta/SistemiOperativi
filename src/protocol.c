/**
 * \file protocol.c
 *
 * \brief Basic protocol for message exchange over pipe - implementation
 */

#include "protocol.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct unitnos_protocol_command unitnos_protocol_parse(char *message) {
  struct unitnos_protocol_command command;
  command.command = message;

  char *newl = strchr(message, '\n');
  assert(newl != NULL);
  *newl = '\0';
  char *delim = strchr(message, ':');
  if (delim) {
    command.value = delim + 1;
    *delim = '\0';
  }

  return command;
}
