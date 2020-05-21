/**
 * \file logger.h
 *
 * \brief Logger - interface
 *
 * Inspired by https://github.com/armink/EasyLogger
 */
#ifndef UNITNOS_LOGGER_H_
#define UNITNOS_LOGGER_H_

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

#define UNITNOS_LOGGER_LOG_LEVEL UNITNOS_LOGGER_LOG_LVL_DEBUG

void unitnos_logger_log(enum unitnos_logger_log_level lvl, const char *tag,
                        const char *file, const char *func, const long line,
                        const char *format, ...);

#ifndef LOG_TAG
#define LOG_TAG "N/A"
#endif
#define log_error(...)                                                         \
  unitnos_logger_log(UNITNOS_LOGGER_LOG_LVL_ERROR, LOG_TAG, __FILE__,          \
                     __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_warn(...)                                                         \
  unitnos_logger_log(UNITNOS_LOGGER_LOG_LVL_WARN, LOG_TAG, __FILE__,          \
                     __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_info(...)                                                          \
  unitnos_logger_log(UNITNOS_LOGGER_LOG_LVL_INFO, LOG_TAG, __FILE__,           \
                     __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_debug(...)                                                         \
  unitnos_logger_log(UNITNOS_LOGGER_LOG_LVL_DEBUG, LOG_TAG, __FILE__,          \
                     __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_verbose(...)                                                       \
  unitnos_logger_log(UNITNOS_LOGGER_LOG_LVL_VERBOSE, LOG_TAG, __FILE__,        \
                     __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_LOGGER_H_ */
