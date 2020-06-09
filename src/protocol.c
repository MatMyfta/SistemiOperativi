/**
 * \file protocol.c
 *
 * \brief Basic protocol for message exchange over pipe - implementation
 */

#include "protocol.h"

#include "logger.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static sig_atomic_t reception_cnt = 0;
static void sigusr(int signo) {
  ++reception_cnt;
  printf("%u A\n", getpid());
}

int unitnos_procotol_init() {
  /*
   * Use sigaction instead of signal()
   * We don't need to reinstall signal catcher!
   */
  struct sigaction sigact;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_handler = sigusr;
  sigact.sa_flags = 0;
  // System calls interrupted by this signal are automatically restarted.
  sigact.sa_flags |= SA_RESTART;

  if (sigaction(SIGUSR1, &sigact, NULL) < 0) {
    log_error("Unable to set SIGUSR1 signal handler");
    return -1;
  }

  if (sigaction(SIGUSR2, &sigact, NULL) < 0) {
    log_error("Unable to set SIGUSR2 signal handler");
    return -1;
  }
  return 0;
}

void unitnos_procotol_ack(pid_t pid) {
  int ret = kill(pid, SIGUSR2);
  if (ret == -1) {
    log_error("Unable to send signal to process %u", pid);
  }
}

void unitnos_procotol_wait() {
  if (reception_cnt == 0) {
    log_debug("Waiting");
    pause();
  }
  assert(reception_cnt > 0);
  --reception_cnt;
}

void unitnos_procotol_write(int fd, pid_t pid, void *buf, size_t size) {
  write(fd, buf, size);
  int ret = kill(pid, SIGUSR1);
  if (ret == -1) {
    log_error("Unable to send signal to process %u", pid);
  }

  pause();
  log_error("ACK received");
}

void unitnos_procotol_send_command(int fd, pid_t pid, const char *command) {
  char buf[strlen(command) + 1];
  strcpy(buf, command);
  buf[sizeof(buf) - 1] = '\n';
  unitnos_procotol_write(fd, pid, buf, sizeof(buf));
}
void unitnos_procotol_send_command1(unitnos_process *process,
                                    const char *command) {
  unitnos_procotol_send_command(unitnos_process_get_fd(process, "w"),
                                unitnos_process_get_pid(process), command);
}

void unitnos_procotol_send_command_with_binary_data(int fd, pid_t pid,
                                                    const char *command,
                                                    void *data, size_t size) {
  size_t command_len = strlen(command);
  assert(command_len > 0);
  char buf[command_len + 1 + size + 1];
  strcpy(buf, command);
  buf[command_len] = ':';
  strncpy(buf + command_len + 1, data, size);
  buf[command_len + 1 + size] = '\n';
  printf("BUF SIZE: %lu", sizeof(buf));
  unitnos_procotol_write(fd, pid, buf, sizeof(buf));
}

void unitnos_procotol_send_command_with_binary_data1(unitnos_process *process,
                                                     const char *command,
                                                     void *data, size_t size) {
  unitnos_procotol_send_command_with_binary_data(
      unitnos_process_get_fd(process, "w"), unitnos_process_get_pid(process),
      command, data, size);
}

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
