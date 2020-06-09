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
  log_debug("Fetching statistics");

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
    printf("CNT: %lu, re: %ld\n", cnt, ret);
    printf("message_buf: %.*s\n-------------", (int)stat_content_message_size,
           message_buf);
    if (ret == -1) {
      log_error("Unable to read statistics from fd %d: %s", fd,
                strerror(errno));
      return ret;
    }
    cnt += ret;
  }

  struct unitnos_protocol_command cmd = unitnos_protocol_parse(message_buf);
  printf("%s, %s\n", cmd.command, command_name);
  assert(!strcmp(cmd.command, command_name));
  memcpy(stat, cmd.value, sizeof(struct unitnos_char_count_statistics));

  return 0;
}
