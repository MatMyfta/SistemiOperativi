/**
 * \file list.h
 *
 * \brief Generic doubly linked list - interface
 */

#ifndef UNITNOS_LIST_H_
#define UNITNOS_LIST_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list list;
typedef struct list_node list_node;

/**
 * Use as opaque type
 */
struct list_node {
  list_node *prev;
  list_node *next;
  void *data;
};

/**
 * Use as opaque type
 */
struct list {
  list_node *head;
  list_node *tail;
  size_t size;
  size_t element_size;
};

list *list_create(size_t element_size);
void list_destroy(list *list);
/**
 * Access last element
 */
void *list_back(list *list);
/**
 * Add element at the end
 */
void list_push_back(list *list, const void *data);
/**
 * Delete last element
 */
void list_pop_back(list *list);
/**
 * Find the node that holds \p data and delete it
 *
 * Complexity: O(n)
 */
void list_remove(list *list, void *data);
/**
 * Remove a list node
 *
 * Complexity: O(1)
 */
void list_remove_node(list *list, list_node *node);
/**
 * Get the number of items in the list
 *
 * Complexity: O(1)
 */
size_t list_size(list *list);

#define list_for_each_node(it, list)                                           \
  for (it = list->head; it != NULL; it = it->next)

#define list_for_each_data(_data_, _type_, _it_, _list_)                       \
  for ((_it_ = _list_->head) && (_data_ = _it_->data); _it_ != NULL;           \
       (_it_ = _it_->next) && (_data_ = (_type_ *)_it_->data))

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_LIST_H_ */
