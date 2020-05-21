/**
 * \file logger.c
 *
 * \brief Logger - implementation
 *
 * Inspired by https://github.com/armink/EasyLogger
 */
#include "logger.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/*******************************************************************************
 * Private function declarations
 ******************************************************************************/
bool is_color_enabled(enum unitnos_logger_log_level lvl);
bool is_fmt_enabled(enum unitnos_logger_log_level lvl,
                    enum unitnos_logger_fmt fmt);
char *get_time(void);
char *get_pinfo(void);
/**
 * Safe strncpy from index
 *
 * \returns copied length
 */
size_t safe_strcpy_from(size_t index, char *dest, size_t destsz,
                        const char *src);
size_t logbufcpy(size_t index, char *dest, const char *src);

/*******************************************************************************
 * Private variable & constants
 ******************************************************************************/
#define LOG_LINE_LENGTH (1024 + 1)
#define LOG_MAX_SOURCE_FILE_LINE_DIGITS (6)
#define NEW_LINE_SIGN ("\n")

/**
 * CSI(Control Sequence Introducer/Initiator) sign
 * more information on https://en.wikipedia.org/wiki/ANSI_escape_code
 */
#define CSI_START "\033["
#define CSI_END "\033[0m"
#define ANSI_TEXT_COLOR_BLACK "30"
#define ANSI_TEXT_COLOR_RED "31"
#define ANSI_TEXT_COLOR_GREEN "32"
#define ANSI_TEXT_COLOR_YELLOW "33"
#define ANSI_TEXT_COLOR_BLUE "34"
#define ANSI_TEXT_COLOR_MAGENTA "35"
#define ANSI_TEXT_COLOR_CYAN "36"
#define ANSI_TEXT_COLOR_WHITE "37"

const char *LEVEL2TEXT[] = {
    [UNITNOS_LOGGER_LOG_LVL_ERROR] = "ERROR",
    [UNITNOS_LOGGER_LOG_LVL_WARN] = "WARN",
    [UNITNOS_LOGGER_LOG_LVL_INFO] = "INFO",
    [UNITNOS_LOGGER_LOG_LVL_DEBUG] = "DEBUG",
    [UNITNOS_LOGGER_LOG_LVL_VERBOSE] = "VERBOSE",
};

const char *LEVEL2COLOR[] = {
    [UNITNOS_LOGGER_LOG_LVL_ERROR] = ANSI_TEXT_COLOR_RED "m",
    [UNITNOS_LOGGER_LOG_LVL_WARN] = ANSI_TEXT_COLOR_YELLOW "m",
    [UNITNOS_LOGGER_LOG_LVL_INFO] = ANSI_TEXT_COLOR_GREEN "m",
    [UNITNOS_LOGGER_LOG_LVL_DEBUG] = ANSI_TEXT_COLOR_CYAN "m",
    [UNITNOS_LOGGER_LOG_LVL_VERBOSE] = ANSI_TEXT_COLOR_MAGENTA "m",
};

static struct {
  uint16_t formats[UNITNOS_LOGGER_LOG_LVL_TOTAL_LEVELS];
} g_logger = {
    .formats =
        {
            [UNITNOS_LOGGER_LOG_LVL_ERROR] =
                UNITNOS_LOGGER_LOG_FMT_LVL | UNITNOS_LOGGER_LOG_FMT_TAG,
            [UNITNOS_LOGGER_LOG_LVL_WARN] = UNITNOS_LOGGER_LOG_FMT_LVL,
            [UNITNOS_LOGGER_LOG_LVL_INFO] = 0,
            [UNITNOS_LOGGER_LOG_LVL_DEBUG] =
                UNITNOS_LOGGER_LOG_FMT_LVL | UNITNOS_LOGGER_LOG_FMT_TAG |
                UNITNOS_LOGGER_LOG_FMT_PROCESS_INFO,
            [UNITNOS_LOGGER_LOG_LVL_VERBOSE] =
                UNITNOS_LOGGER_LOG_FMT_LVL | UNITNOS_LOGGER_LOG_FMT_TAG |
                UNITNOS_LOGGER_LOG_FMT_PROCESS_INFO |
                UNITNOS_LOGGER_LOG_FMT_TIME,
        },
};

void unitnos_logger_log(enum unitnos_logger_log_level lvl, const char *tag,
                        const char *file, const char *func, const long line,
                        const char *format, ...) {
  static char g_log_buffer[LOG_LINE_LENGTH];

  size_t current_index = 0;

  /* package color */
  if (is_color_enabled(lvl)) {
    current_index +=
        logbufcpy(current_index, g_log_buffer + current_index, CSI_START);
    current_index += logbufcpy(current_index, g_log_buffer + current_index,
                               LEVEL2COLOR[lvl]);
  }

  /* package level info */
  if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_LVL)) {
    current_index +=
        logbufcpy(current_index, g_log_buffer + current_index, LEVEL2TEXT[lvl]);
    current_index +=
        logbufcpy(current_index, g_log_buffer + current_index, "/");
  }

  /* package tag info */
  if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_TAG)) {
    current_index +=
        logbufcpy(current_index, g_log_buffer + current_index, tag);
    current_index +=
        logbufcpy(current_index, g_log_buffer + current_index, " ");
  }

  /* package time and process info */
  if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_TIME |
                              UNITNOS_LOGGER_LOG_FMT_PROCESS_INFO)) {
    current_index +=
        logbufcpy(current_index, g_log_buffer + current_index, "[");
    /* package time info */
    if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_TIME)) {
      current_index +=
          logbufcpy(current_index, g_log_buffer + current_index, get_time());
      if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_PROCESS_INFO)) {
        current_index +=
            logbufcpy(current_index, g_log_buffer + current_index, "; ");
      }
    }
    if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_PROCESS_INFO)) {
      current_index +=
          logbufcpy(current_index, g_log_buffer + current_index, get_pinfo());
    }
    current_index +=
        logbufcpy(current_index, g_log_buffer + current_index, "] ");
  }

  /* package file directory and name, function name and line number info */
  if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_FILE |
                              UNITNOS_LOGGER_LOG_FMT_FUNC |
                              UNITNOS_LOGGER_LOG_FMT_LINE)) {
    current_index +=
        logbufcpy(current_index, g_log_buffer + current_index, "(");
    /* package file info */
    if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_FILE)) {
      current_index +=
          logbufcpy(current_index, g_log_buffer + current_index, file);
      if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_FUNC)) {
        current_index +=
            logbufcpy(current_index, g_log_buffer + current_index, " ");
      } else if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_LINE)) {
        current_index +=
            logbufcpy(current_index, g_log_buffer + current_index, ":");
      }
    }
    /* package func info */
    if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_FUNC)) {
      current_index +=
          logbufcpy(current_index, g_log_buffer + current_index, func);
      if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_LINE)) {
        current_index +=
            logbufcpy(current_index, g_log_buffer + current_index, ":");
      }
    }
    /* package line info */
    if (is_fmt_enabled(lvl, UNITNOS_LOGGER_LOG_FMT_LINE)) {
      char line_num[LOG_MAX_SOURCE_FILE_LINE_DIGITS + 1] = {0};
      snprintf(line_num, LOG_MAX_SOURCE_FILE_LINE_DIGITS, "%ld", line);
      current_index +=
          logbufcpy(current_index, g_log_buffer + current_index, line_num);
    }
    current_index +=
        logbufcpy(current_index, g_log_buffer + current_index, ")");
  }

  va_list args;
  va_start(args, format);
  /* package other log data to buffer. '\0' must be added in the end by
   * vsnprintf. */
  int potentially_written =
      vsnprintf(g_log_buffer + current_index, LOG_LINE_LENGTH - current_index,
                format, args);
  va_end(args);

  if (is_color_enabled(lvl)) {
    if (current_index + potentially_written + (sizeof(CSI_END) - 1) + 1 <=
        LOG_LINE_LENGTH) {
      current_index += potentially_written;
    } else {
      current_index = LOG_LINE_LENGTH - ((sizeof(CSI_END) - 1) + 1);
    }
    current_index +=
        logbufcpy(current_index, g_log_buffer + current_index, CSI_END);
  } else {
    if (current_index + potentially_written < LOG_LINE_LENGTH) {
      current_index += potentially_written;
    } else {
      current_index = LOG_LINE_LENGTH - 1;
    }
  }
  g_log_buffer[current_index] = '\0';

  // Finally, print
  if (lvl > UNITNOS_LOGGER_LOG_LVL_INFO) {
    puts(g_log_buffer);
  } else {
    fputs(g_log_buffer, stderr);
    fputs("\n", stderr);
  }
}

/*******************************************************************************
 * Private function definitions
 ******************************************************************************/

bool is_fmt_enabled(enum unitnos_logger_log_level lvl,
                    enum unitnos_logger_fmt fmt) {
  return g_logger.formats[lvl] & fmt;
}
bool is_color_enabled(enum unitnos_logger_log_level lvl) {
  return isatty(fileno(stdout)) && LEVEL2COLOR[lvl] != NULL;
}

size_t safe_strcpy_from(size_t index, char *dest, size_t destsz,
                        const char *src) {
  assert(index < destsz);
  assert(dest);
  assert(src);
  const char *src_old = src;
  while (*src != '\0') {
    /* make sure destination has enough space */
    if (index++ < destsz) {
      *dest++ = *src++;
    } else {
      break;
    }
  }
  return src - src_old;
}

size_t logbufcpy(size_t index, char *dest, const char *src) {
  return safe_strcpy_from(index, dest, LOG_LINE_LENGTH, src);
}

char *get_time(void) {
  static char buf[24] = {0};

  time_t timep;

  time(&timep);
  struct tm *p = localtime(&timep);
  if (p == NULL) {
    return "";
  }
  strftime(buf, sizeof buf, "%H:%M:%S", p);
  return buf;
}

char *get_pinfo(void) {
  static char str[64];
  snprintf(str, 64, "ppid: %d, pid: %d", getppid(), getpid());
  return str;
}
