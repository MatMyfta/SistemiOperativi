/**
 * \file path.h
 *
 * Helper functions for path "unpacking"
 */

#ifndef UNITNOS_COUNTER_PATH_H_
#define UNITNOS_COUNTER_PATH_H_

#include "../../containers/set.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Given \p path, it returns an instance of unitnos_unpacked_paths, where
 * unitnos_unpacked_paths::files is filled with
 *
 * * paths to all the files present in the given \p path, in case it's a path
 * to a directory
 * * a single path to the same file, in case it's a path to a file
 *
 * \returns instance of unitnos_unpacked_paths
 * \retval NULL if the given path is invalid
 */
unitnos_set *unitnos_unpack_path(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_COUNTER_PATH_H_ */
