#include "base.h"

#include <stdlib.h>
#include <string.h>

int unitnos_container_util_strcmp(const void *lhs, const void *rhs,
                                  void *user_data) {
  const char *lhs_str = (const char *)lhs;
  const char *rhs_str = (const char *)rhs;
  return strcmp(lhs_str, rhs_str);
}

/**
 * Wrapper of free() that can be used as unitnos_destroy_nodify
 */
void unitnos_container_util_free(void *data, void *user_data) { free(data); }
