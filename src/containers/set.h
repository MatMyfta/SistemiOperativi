/**
 * \file set.h
 *
 * \brief Sorted set - interface
 *
 * A set built upon a tree
 */
#ifndef UNITNOS_SET_H_
#define UNITNOS_SET_H_

#include "../bool.h"
#include "base.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct unitnos_set unitnos_set;

/**
 * Specifies the type of function passed to #unitnos_set_foreach(). It is
 * passed the value of each item present in the set, together with the optional
 * user_data parameter passed to #unitnos_set_foreach(). If the function
 * returns true, the traversal is stopped.
 */
typedef bool (*unitnos_set_transverse_func)(void *value, void *user_data);

/**
 * Creates a new set and allows to specify functions to free the memory
 * allocated for item that get called when removing the entry from the set.
 */
unitnos_set *unitnos_set_create(unitnos_compare_func compare_func,
                                unitnos_destroy_nodify value_destroy_func,
                                void *user_data);
/**
 * Destroy the set
 */
void unitnos_set_destroy(unitnos_set *set);
/**
 * Insert the given value in the set. If the value is already present, nothing
 * happens.
 */
void unitnos_set_insert(unitnos_set *set, void *value);
/**
 * Remove the given value from the set. If the value is not present, nothing
 * happens.
 */
void unitnos_set_remove(unitnos_set *set, const void *value);

/**
 * Calls the given function for each of the node in the set. The function is
 * passed value of each node, and the given data parameter. The set is
 * traversed in sorted order.
 *
 * The set may not be modified while iterating over it (you can't add/remove
 * items). To remove all items matching a predicate, you need to add each item
 * to a list in your unitnos_set_transverse_funcFunc as you walk over the
 * set, then walk the list and remove each item.
 *
 * \param [in] set a set
 * \param [in] func the function to call for each node visited. If this
 * function returns true, the traversal is stopped.
 * \param [in] user_data user data to pass to the function
 */
void unitnos_set_foreach(unitnos_set *set, unitnos_set_transverse_func func,
                         void *user_data);

/**
 * Get the number of elements in the set
 */
size_t unitnos_set_size(unitnos_set *set);
/**
 * Check whether the given value is present in the set
 */
bool unitnos_set_contains(unitnos_set *set, const void *value);
/**
 * Check whether a value equivalent to the given value exists in the set.
 *
 * \returns the original value
 */
void *unitnos_set_lookup(unitnos_set *set, const void *value);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_SET_H_ */
