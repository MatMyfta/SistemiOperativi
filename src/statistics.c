#include "statistics.h"

#include "logger.h"
#include "protocol.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int unitnos_char_count_statistics_read(
    const char *command_name, struct unitnos_char_count_statistics *stat,
    int fd) {
  size_t stat_content_message_size =
      strlen(command_name) +
      // delimiter
      1 +
      sizeof(struct unitnos_char_count_statistics)
      // newline
      + 1;

  char message_buf[stat_content_message_size];

  size_t cnt = 0;
  while (cnt != stat_content_message_size) {
    ssize_t ret = read(fd, message_buf + cnt, stat_content_message_size - cnt);
    if (ret == -1) {
      if (errno != EINTR) {
        log_error("Unable to read statistics from fd %d: %s", fd,
                  strerror(errno));
        return ret;
      } else {
        continue;
      }
    }
    cnt += ret;
  }

  struct unitnos_protocol_command cmd = unitnos_protocol_parse_binary(
      message_buf, sizeof(struct unitnos_char_count_statistics));
  assert(!strcmp(cmd.command, command_name));
  memcpy(stat, cmd.value, sizeof(struct unitnos_char_count_statistics));
  return 0;
}
