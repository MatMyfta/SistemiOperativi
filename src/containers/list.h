/**
 * \file list.h
 *
 * \brief Generic doubly linked list - interface
 */

#ifndef UNITNOS_LIST_H_
#define UNITNOS_LIST_H_

#include "../bool.h"
#include "base.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct unitnos_list unitnos_list;
typedef struct unitnos_list_node unitnos_list_node;
/**
 * Specifies the type of function passed to #unitnos_list_foreach(). It is
 * passed the value of each item present in the list, together with the
 * optional user_data parameter passed to #unitnos_list_foreach(). If the
 * function returns true, the traversal is stopped.
 */
typedef bool (*unitnos_list_transverse_func)(void *value, void *user_data);

/**
 * Create a list
 */
unitnos_list *unitnos_list_create(unitnos_destroy_nodify value_destroy_func,
                                  void *user_data);
void unitnos_list_destroy(unitnos_list *list);
/**
 * Access last element
 */
void *unitnos_list_back(unitnos_list *list);
/**
 * Add element at the end
 */
void unitnos_list_push_back(unitnos_list *list, void *data);
/**
 * Delete last element
 */
void unitnos_list_pop_back(unitnos_list *list);
/**
 * Find the node that holds \p data and delete it
 *
 * Complexity: O(n)
 */
void unitnos_list_remove(unitnos_list *list, const void *data);
/**
 * Get the number of items in the list
 *
 * Complexity: O(1)
 */
size_t unitnos_list_size(unitnos_list *list);

/**
 * Calls the given function for each of the item in the list. The function is
 * passed value of each item, and the given user_data parameter.
 *
 * The list may not be modified while iterating over it (you can't add/remove
 * items).
 *
 * \param [in] list a list
 * \param [in] func the function to call for each item visited. If this
 * function returns true, the traversal is stopped.
 * \param [in] user_data user data to pass to the function
 */
void unitnos_list_foreach(unitnos_list *list, unitnos_list_transverse_func func,
                          void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_LIST_H_ */
