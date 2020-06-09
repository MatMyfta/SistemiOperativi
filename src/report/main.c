#include "report.h"

#include "../bool.h"
#include "../logger.h"

#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct command {
  const char *name;
  const char *help;
  int (*function)(int argc, const char *argv[]);
};

static void quit();
static int help_command(int argc, const char *argv[]);
static int send_request_command(int argc, const char *argv[]);
static void command_ask_global(const char *type);
static void command_ask_path(const char *type);

static int send_ack(char *value);
static bool is_ack_end(const char *str);
static int send_message(char *message_to_send);
static char *read_message();
char *concat(const char *s1, const char *s2);

struct command g_commands[] = {
    {
        .name = "help",
        .help = "",
        .function = help_command,
    },
    {
        .name = "full_print",
        .help = "Print all statistcs",
        .function = NULL,
    },
    {
        .name = "partial_print",
        .help = "Print only statistcs different from 0",
        .function = NULL,
    },
    {
        .name = "send_request",
        .help = "Ask to Analyzer for statistcs",
        .function = send_request_command,
    },
    {
        .name = "quit",
        .help = "Quit",
        .function = NULL,
    },

};

/*******************************************************************************
 * Command parser
 *******************************************************************************/
/**
 * Splits the given \p line into tokens, using whitespace as separator, and
 * fills them in the given \p *argv of size \p *n, automatically increasing its
 * size as if by realloc to fit all the tokens.
 * \p *argv may be null, in which case *n is ignored and this function allocates
 * the appropriate amount to space.
 *
 * \param [in,out] line a string with terminated with '\n\0'
 * \param [out] argc pointer to argv
 * \param [out] argv parsed argv
 * \param [in,out] n size of argv
 */
static void parse(char *line, int *argc, const char ***argv, size_t *n);

int main() {
  mkfifo(unitnos_fifo, 0666);

  int unitnos_report_records[MAXDIM];
  unitnos_report_record_init(unitnos_report_records);
  printf("Waiting for statistics input\n");

  printf("Statistics found, now formatting\n");
  unitnos_report_record_fill(unitnos_report_records);
  printf("Reported statistics:\n\n");

  int total_char = unitnos_report_total_count(unitnos_report_records);

  char *command = NULL;
  size_t command_size = 0;

  int command_argc = 0;
  size_t command_argv_size = 0;
  const char **command_argv = NULL;

  while (1) {
    printf("> ");
    if (getline(&command, &command_size, stdin) >= 0) {
      parse(command, &command_argc, &command_argv, &command_argv_size);
      if (command_argc < 1) {
        // invalid command
        continue;
      }
      if (strcmp(g_commands[1].name, command_argv[0]) == 0) {
        unitnos_report_full_print(unitnos_report_records, total_char);
      } else if (strcmp(g_commands[2].name, command_argv[0]) == 0) {
        unitnos_report_partial_print(unitnos_report_records, total_char);
      } else if (strcmp(g_commands[4].name, command_argv[0]) == 0) {
        quit();
      } else {
        size_t i;
        for (i = 0; i < sizeof(g_commands) / sizeof(g_commands[0]); ++i) {
          if (strcmp(g_commands[i].name, command_argv[0]) == 0) {
            g_commands[i].function(command_argc, command_argv);
            break;
          }
        }
        if (i == sizeof(g_commands) / sizeof(g_commands[0])) {
          printf("Unrecognized command \"%s\"\n", command_argv[0]);
        }
      }
    } else {
      if (feof(stdin)) {
        printf("Main EOF. Terminate\n");
        break;
      }
    }
  }
  return 0;
}

/*******************************************************************************
 * Command parser
 *******************************************************************************/
static void parse(char *line, int *argc, const char ***argv, size_t *n) {
  *argc = 0;
  if (*argv == NULL) {
    *n = 0;
  }

  {
    size_t token_cnt = 1;
    size_t i;
    for (i = 0; line[i] != '\n'; ++i) {
      if (line[i] == ' ') {
        /*
         * A white space doesn't necessarily imply another token (e.g. 'a
         * b'), but a bit of memory wastage is acceptable. We just need to
         * ensure that the upper bound is allocated and let strtok handle the
         * nitty-gritty details of tokenization.
         */
        ++token_cnt;
      }
    }
    if (token_cnt > *n) {
      *argv = realloc(*argv, token_cnt * sizeof(const char *));
    }
    // set '\n' to '\0'
    line[i] = '\0';
  }

  char *token = strtok(line, " ");
  size_t i = 0;
  for (; token; ++i) {
    (*argv)[i] = token;
    token = strtok(NULL, " ");
  }
  *argc = i;
}

/*******************************************************************************
 * Commands
 *******************************************************************************/
static void quit() {
  printf("%s\n", "Terminating report");
  exit(0);
}

static int help_command(int argc, const char *argv[]) {
  assert(strcmp(g_commands[0].name, "help") == 0);

  printf("Available commands:\n\n");

  size_t i;
  for (i = 0; i < sizeof(g_commands) / sizeof(g_commands[0]); ++i) {
    printf("%s: %s\n", g_commands[i].name, g_commands[i].help);
  }
  return 0;
}

static int send_request_command(int argc, const char *argv[]) {
  if (argc != 3) {
    printf("%s\n", "Error, usage: send_request <scope> <type>");
  } else if ((strcmp(argv[1], "global") != 0) &&
             (strcmp(argv[1], "path") != 0)) {
    printf("%s | You inserted: [%s]\n", "scope: [global] - [path]", argv[1]);
  } else if ((atoi(argv[2]) < 1) || (atoi(argv[2]) > 4)) {
    printf("%s | You inserted: [%s]\n", "type: [1-4]", argv[2]);
  } else {
    if (strcmp(argv[1], "global") == 0)
      command_ask_global(argv[2]);
    else
      command_ask_path(argv[2]);
  }
  return 0;
}

static void command_ask_global(const char *type) {
  int fd, readed, size = 80;

  /* messaggio viene codificato come comando per analyzer*/
  char *tmp = "global:";
  char *tmp1 = concat(tmp, type);
  char *message_to_send = concat(tmp1, "\n");
  message_to_send[strlen(message_to_send)] = '\0';

  send_message(message_to_send);
  char *response = read_message(size);
  if (response != NULL) // ELABORA RISPOSTA
    printf("%s\n", response);
}

static void command_ask_path(const char *type) {
  /* messaggio viene codificato come comando per analyzer*/
  char *tmp = "path:";
  char *tmp1 = concat(tmp, type);
  char *message_to_send = concat(tmp1, "\n");
  message_to_send[strlen(message_to_send)] = '\0';

  send_message(message_to_send);

  // Mi metto in ascolto finche non ho elaborato ciascun path
  bool received_last_path = false;
  int i = 0;
  while (!received_last_path) {
    char *received_message_path = read_message(MAX_SIZE_MESSAGE);
    char *received_message_stat;
    if (received_message_path != NULL) {
      if (!is_ack_end(received_message_path)) {
        send_ack("1");
        received_message_stat = read_message(MAX_SIZE_MESSAGE);
        if (received_message_stat != NULL) {
          if (!is_ack_end(received_message_stat)) { // ELABORA STATISTICHE
            printf("path #%d received\n", i + 1);
            i++;
            printf("%s - %s\n", received_message_path, received_message_stat);
            send_ack("1");
          } else {
            received_last_path = true;
          }
        } else {
          send_ack("0");
        }
      } else {
        received_last_path = true;
      }
    } else {
      send_ack("0");
    }
  }
}

static int send_message(char *message_to_send) {
  int fd, ret_value;
  fd = open(unitnos_fifo, O_WRONLY);
  if (fd == -1) {
    log_error("Failed opening named pipe");
    ret_value = 1;
  } else {
    if (write(fd, message_to_send, strlen(message_to_send) + 1) == -1) {
      log_error("Failed writing on named pipe");
      ret_value = 1;
    } else {
      ret_value = 0;
    }
    close(fd);
  }
  return ret_value;
}

static char *read_message(int size) {
  char *ret_value;
  int fd;
  ssize_t readed;
  char *message_to_read = malloc(size + 1);
  fd = open(unitnos_fifo, O_RDONLY);
  if (fd == -1) {
    log_error("Failed opening named pipe");
    free(message_to_read);
    ret_value = NULL;
  } else {
    readed = read(fd, message_to_read, size + 1);
    if (readed == -1) {
      log_error("Failed writing on named pipe");
      free(message_to_read);
      ret_value = NULL;
    } else {
      message_to_read[readed] = '\0';
      ret_value = message_to_read;
    }
    close(fd);
  }
  return ret_value;
}

/*
 * value = 1 -> go on, value = 0 -> stop
 */
static int send_ack(char *value) {
  int ret_value;
  char *tmp = "ack:";
  char *tmp1 = concat(tmp, value);
  char *message_to_send = concat(tmp1, "\n");
  message_to_send[(strlen(message_to_send))] = '\0';
  send_message(message_to_send);
  return ret_value;
}

static bool is_ack_end(const char *str) {
  bool ret_value = false;
  if (strcmp(str, ack_end) == 0)
    ret_value = true;
  return ret_value;
}

char *concat(const char *s1, const char *s2) {
  char *result =
      malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}
