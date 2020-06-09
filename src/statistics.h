#ifndef UNITNOS_STATISTICS_H_
#define UNITNOS_STATISTICS_H_

#include <ctype.h>
#include <limits.h>

#if USHRT_MAX == 4294967295
typedef unsigned short int uint32_t;
#elif UINT_MAX == 4294967295
typedef unsigned int uint32_t;
#elif ULONG_MAX == 4294967295
typedef unsigned long uint32_t;
#elif ULLONG_MAX == 4294967295
typedef unsigned long long uint32_t;
#else
#error "Cannot find 32bit integer."
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum unitnos_statistics_type {
  UNITNOS_STATISTICS_TYPE_CHAR_COUNT,
  UNITNOS_STATISTICS_TYPE_CHAR_PERCENTAGE,
  UNITNOS_STATISTICS_TYPE_CHARTYPE_COUNT,
  UNITNOS_STATISTICS_TYPE_CHARTYPE_PERCENTAGE,
};

struct unitnos_char_count_statistics {
  // one counter for each extended-ASCII character
  uint32_t counts[255];
};
struct unitnos_char_percentage_statistics {
  // one percentage for each extended-ASCII character
  unsigned char percentages[255];
};
struct unitnos_chartype_count_statistics {
  uint32_t digit;
  uint32_t lower;
  uint32_t upper;
  uint32_t space;
  uint32_t cntrl;
  uint32_t print;
  uint32_t punct;
};
struct unitnos_chartype_percentage_statistics {
  unsigned char digit;
  unsigned char lower;
  unsigned char upper;
  unsigned char space;
  unsigned char cntrl;
  unsigned char print;
  unsigned char punct;
};

/**
 * Read a message from the given \p fd which is supposed to contain a
 * statistics. The message may be prefixed with \p command and
 * UNITNOS_PROTOCOL_COMMAND_VALUE_DELIMITATOR
 *
 * \param [in] command prepended before the statistics binary string
 * \param [out] stat buffer were the read status will be written
 * \param [in] fd input file descriptor
 *
 * We need this function because `getline()` doesn't work, since the binary
 * string statistics could contain anything (including \0, \n, etc)...
 *
 * \retval 0 OK
 * \retval -1 error
 */
int unitnos_char_count_statistics_read(
    const char *command, struct unitnos_char_count_statistics *stat, int fd);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_STATISTICS_H_ */
