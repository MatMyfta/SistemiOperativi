/**
 * \file logger.h
 *
 * \brief Logger - interface
 *
 * Inspired by https://github.com/armink/EasyLogger
 */
#ifndef UNITNOS_LOGGER_H_
#define UNITNOS_LOGGER_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

enum unitnos_logger_fmt {
  UNITNOS_LOGGER_LOG_FMT_LVL = 1,
  UNITNOS_LOGGER_LOG_FMT_TAG = 1 << 1,
  UNITNOS_LOGGER_LOG_FMT_TIME = 1 << 2,
  UNITNOS_LOGGER_LOG_FMT_PROCESS_INFO = 1 << 3,
  UNITNOS_LOGGER_LOG_FMT_FILE = 1 << 4,
  UNITNOS_LOGGER_LOG_FMT_FUNC = 1 << 4,
  UNITNOS_LOGGER_LOG_FMT_LINE = 1 << 5,
};

enum unitnos_logger_log_level {
  UNITNOS_LOGGER_LOG_LVL_ERROR,
  UNITNOS_LOGGER_LOG_LVL_WARN,
  UNITNOS_LOGGER_LOG_LVL_INFO,
  UNITNOS_LOGGER_LOG_LVL_DEBUG,
  UNITNOS_LOGGER_LOG_LVL_VERBOSE,
  UNITNOS_LOGGER_LOG_LVL_TOTAL_LEVELS,
};

#define UNITNOS_LOGGER_LOG_LVL_ERROR 0
#define UNITNOS_LOGGER_LOG_LVL_WARN 1
#define UNITNOS_LOGGER_LOG_LVL_INFO 2
#define UNITNOS_LOGGER_LOG_LVL_DEBUG 3
#define UNITNOS_LOGGER_LOG_LVL_VERBOSE 4

#define UNITNOS_LOGGER_LOG_LEVEL UNITNOS_LOGGER_LOG_LVL_INFO

/**
 * Build to initial portion of the log string
 *
 * \param [out] log_string pointer to the start of the remaining portion of the
 * log line buffer, after all the ancillary information (color, level, file,
 * etc.) has be written.
 *
 * \returns remaining bytes in the log string
 */
size_t unitnos_logger_log_start(char **log_string,
                                enum unitnos_logger_log_level lvl,
                                const char *tag, const char *file,
                                const char *func, long line);
/**
 * "Close" the log string and print it
 *
 * \param [in] log_string the remaining portion of log string after snprintf
 * \param [in] remaining_size remaining size in the log string buffer.
 * For how unitnos_logger_log macro is implemented, we assume that its value is
 * >= 1.
 */
void unitnos_logger_log_end(char *log_string, size_t remaining_size,
                            enum unitnos_logger_log_level lvl);

#define unitnos_logger_log(level, tag, file, func, line, args...)              \
  do {                                                                         \
    char *log_line;                                                            \
    size_t remaining =                                                         \
        unitnos_logger_log_start(&log_line, level, tag, file, func, line);     \
    size_t potentially_written = snprintf(log_line, remaining, args);          \
    /* how many characters has snprintf without counting the final '\0' */     \
    size_t written = potentially_written < remaining - 1 ? potentially_written \
                                                         : remaining - 1;      \
    unitnos_logger_log_end(log_line + written, remaining - written, level);    \
  } while (0)

#ifndef LOG_TAG
#define LOG_TAG "N/A"
#endif

#if UNITNOS_LOGGER_LOG_LEVEL >= UNITNOS_LOGGER_LOG_LVL_ERROR
#define log_error(...)                                                         \
  unitnos_logger_log(UNITNOS_LOGGER_LOG_LVL_ERROR, LOG_TAG, __FILE__,          \
                     __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_error(...) ((void)0)
#endif

#if UNITNOS_LOGGER_LOG_LEVEL >= UNITNOS_LOGGER_LOG_LVL_WARN
#define log_warn(...)                                                          \
  unitnos_logger_log(UNITNOS_LOGGER_LOG_LVL_WARN, LOG_TAG, __FILE__,           \
                     __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_warn(...) ((void)0)
#endif

#if UNITNOS_LOGGER_LOG_LEVEL >= UNITNOS_LOGGER_LOG_LVL_INFO
#define log_info(...)                                                          \
  unitnos_logger_log(UNITNOS_LOGGER_LOG_LVL_INFO, LOG_TAG, __FILE__,           \
                     __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_info(...) ((void)0)
#endif

#if UNITNOS_LOGGER_LOG_LEVEL >= UNITNOS_LOGGER_LOG_LVL_DEBUG
#define log_debug(...)                                                         \
  unitnos_logger_log(UNITNOS_LOGGER_LOG_LVL_DEBUG, LOG_TAG, __FILE__,          \
                     __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_debug(...) ((void)0)
#endif

#if UNITNOS_LOGGER_LOG_LEVEL >= UNITNOS_LOGGER_LOG_LVL_VERBOSE
#define log_verbose(...)                                                       \
  unitnos_logger_log(UNITNOS_LOGGER_LOG_LVL_VERBOSE, LOG_TAG, __FILE__,        \
                     __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_verbose(...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_LOGGER_H_ */
