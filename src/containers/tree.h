/**
 * \file tree.h
 *
 * \brief Generic BST - interface
 *
 * It can be used to build more advanced/specific data structures.
 *
 * It can be used as a set.
 */
#ifndef UNITNOS_TREE_H_
#define UNITNOS_TREE_H_

#include "../bool.h"
#include "base.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct unitnos_tree unitnos_tree;

/**
 * Specifies the type of function passed to #unitnos_tree_foreach(). It is
 * passed the value of each node, together with the optional user_data
 * parameter passed to #unitnos_tree_foreach(). If the function returns true,
 * the traversal is stopped.
 */
typedef bool (*unitnos_tree_transverse_func)(void *value, void *user_data);

/**
 * Creates a new tree like and allows to specify functions to free the memory
 * allocated for the value that get called when removing the entry from the
 * tree.
 */
unitnos_tree *unitnos_tree_create(unitnos_compare_func compare_func,
                                  unitnos_destroy_nodify value_destroy_func,
                                  void *user_data);
/**
 * Destroy the tree.
 */
void unitnos_tree_destroy(unitnos_tree *tree);
/**
 * Insert the given value in the tree. If the value is already present, nothing
 * happens.
 */
void unitnos_tree_insert(unitnos_tree *tree, void *value);
/**
 * Remove the given value from the tree. If the value is not present, nothing
 * happens.
 */
void unitnos_tree_remove(unitnos_tree *tree, const void *value);

/**
 * Calls the given function for each of the node in the tree. The function is
 * passed value of each node, and the given data parameter. The tree is
 * traversed in sorted order.
 *
 * The tree may not be modified while iterating over it (you can't add/remove
 * items). To remove all items matching a predicate, you need to add each item
 * to a list in your unitnos_tree_transverse_funcFunc as you walk over the
 * tree, then walk the list and remove each item.
 *
 * \param [in] tree a tree
 * \param [in] func the function to call for each node visited. If this
 * function returns true, the traversal is stopped.
 * \param [in] user_data user data to pass to the function
 */
void unitnos_tree_foreach(unitnos_tree *tree, unitnos_tree_transverse_func func,
                          void *user_data);

/**
 * Get the number of elements in the tree
 */
size_t unitnos_tree_size(unitnos_tree *tree);
/**
 * Check whether the given value is present in the tree
 */
bool unitnos_tree_contains(unitnos_tree *tree, const void *value);
/**
 * Check whether a value equivalent to the given value exists in the tree.
 *
 * \returns the original value
 */
void *unitnos_tree_lookup(unitnos_tree *tree, const void *value);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_TREE_H_ */
