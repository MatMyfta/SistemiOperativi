#define LOG_TAG "main"
#include "../analyzer/analyzer.h"
#include "../logger.h"
#include "../protocol.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unitnos_analyzer *g_analyzer;

/*******************************************************************************
 * Commands
 *******************************************************************************/
struct command {
  const char *name;
  const char *help;
  int (*function)(int argc, const char *argv[]);
};

static int help_command(int argc, const char *argv[]);
static int set_n_command(int argc, const char *argv[]);
static int set_m_command(int argc, const char *argv[]);
static int add_new_path_command(int argc, const char *argv[]);
static int list_paths_command(int argc, const char *argv[]);
static int status_panel_command(int argc, const char *argv[]);

struct command g_commands[] = {
    {
        .name = "help",
        .help = "",
        .function = help_command,
    },
    {
        .name = UNITNOS_ANALYZER_COMMAND_ADD_NEW_PATH,
        .help = "Add another path to analyze. Usage: add_new_path <path>",
        .function = add_new_path_command,
    },
    {
        .name = UNITNOS_ANALYZER_COMMAND_LIST_PATHS,
        .help = "Add another path to analyze. Usage: list_paths",
        .function = list_paths_command,
    },
    {
        .name = UNITNOS_ANALYZER_COMMAND_SET_N,
        .help = "Update number of process \"p\". Usage set_n <n>",
        .function = set_n_command,
    },
    {
        .name = UNITNOS_ANALYZER_COMMAND_SET_M,
        .help = "Update number of process \"q\" for each process \"p\". Usage: "
                "set_m <m>",
        .function = set_m_command,
    },
    {
        .name = UNITNOS_ANALYZER_COMMAND_STATUS_PANEL,
        .help = "Overview on the active processes. Usage: status_panel",
        .function = status_panel_command,
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

/*******************************************************************************
 * Public functions implementation
 *******************************************************************************/
static sig_atomic_t g_sigterm_received = 0;
static void sigterm(int signo) { g_sigterm_received++; }

int main() {
  char *command = NULL;
  size_t command_size = 0;

  int command_argc = 0;
  size_t command_argv_size = 0;
  const char **command_argv = NULL;

  if (signal(SIGTERM, sigterm) == SIG_ERR) {
    log_error("Unable to register SIGTERM handler");
    exit(-1);
  }

  if (unitnos_procotol_init() == -1) {
    log_error("Unable to initialize communication protocol");
    exit(-1);
  }

  g_analyzer = unitnos_analyzer_create();
  if (!g_analyzer) {
    log_error("Unable to create analyzer");
    return -1;
  }

  unitnos_analyzer_set_n(g_analyzer, 3);
  unitnos_analyzer_set_m(g_analyzer, 4);

  while (!g_sigterm_received) {
    printf("> ");
    if (getline(&command, &command_size, stdin) >= 0) {
      parse(command, &command_argc, &command_argv, &command_argv_size);
      if (command_argc < 1) {
        // invalid command
        continue;
      }
      size_t i;
      for (i = 0; i < sizeof(g_commands) / sizeof(g_commands[0]); ++i) {
        if (strcmp(g_commands[i].name, command_argv[0]) == 0) {
          g_commands[i].function(command_argc, command_argv);
          break;
        }
      }
      if (i == sizeof(g_commands) / sizeof(g_commands[0])) {
        log_error("Unrecognized command \"%s\"", command_argv[0]);
      }
    } else {
      if (feof(stdin)) {
        log_debug("Main EOF. Terminate");
        break;
      }
    }
  }

  unitnos_analyzer_delete(g_analyzer);
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
static int help_command(int argc, const char *argv[]) {
  assert(strcmp(g_commands[0].name, "help") == 0);

  printf("Available commands:\n\n");

  size_t i;
  for (i = 1; i < sizeof(g_commands) / sizeof(g_commands[0]); ++i) {
    printf("%s: %s\n", g_commands[i].name, g_commands[i].help);
  }
  return 0;
}
static int add_new_path_command(int argc, const char *argv[]) {
  if (argc != 2) {
    log_error("Usage: add_new_path <path>");
    return -1;
  }
  unitnos_analyzer_add_new_path(g_analyzer, argv[1]);
  return 0;
}
static int list_paths_command(int argc, const char *argv[]) {
  if (argc != 1) {
    log_error("Usage: list_paths");
    return -1;
  }
  unitnos_analyzer_list_paths(g_analyzer);
  return 0;
}
static int set_n_command(int argc, const char *argv[]) {
  if (argc != 2) {
    log_error("Usage: set_n <n>");
    return -1;
  }
  char *tmp;
  unsigned long n = strtoul(argv[1], &tmp, 0);
  if (tmp != argv[1]) {
    if (n == 0) {
      log_error("Invalid parameter: n has to be greater than 0");
      return -1;
    }
    unitnos_analyzer_set_n(g_analyzer, n);
  } else {
    log_error("Invalid parameter: not an unsigned integer");
    return -1;
  }
  return 0;
}
static int set_m_command(int argc, const char *argv[]) {
  if (argc != 2) {
    log_error("Usage: set_n <n>");
    return -1;
  }
  char *tmp;
  unsigned long m = strtoul(argv[1], &tmp, 0);
  if (tmp != argv[1]) {
    if (m == 0) {
      log_error("Invalid parameter: m has to be greater than 0");
      return -1;
    }
    unitnos_analyzer_set_m(g_analyzer, m);
  } else {
    log_error("Invalid parameter: not an unsigned integer");
    return -1;
  }
  return 0;
}

static int status_panel_command(int argc, const char *argv[]) {
  if (argc != 1) {
    log_error("Usage: status_panel");
    return -1;
  }
  printf("Main at PID %u\n", getpid());
  unitnos_analyzer_status_panel(g_analyzer);
  return 0;
}
