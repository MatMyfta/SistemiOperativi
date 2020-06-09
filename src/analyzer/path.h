/**
 * \file path.h
 *
 * Helper functions for path "unpacking"
 */

#ifndef UNITNOS_COUNTER_PATH_H_
#define UNITNOS_COUNTER_PATH_H_

#include "../containers/set.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Given \p path, it returns a set filled with:
 *
 * * paths to all the files present in the given \p path, in case it's a path
 * to a directory
 * * a single path to the same file, in case it's a path to a file
 *
 * **NOTE**: The elements of the set are not freed.
 *
 * \returns set of paths
 * \retval NULL if the given path is invalid or any other error
 *
 */
unitnos_set *unitnos_unpack_path(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_COUNTER_PATH_H_ */
