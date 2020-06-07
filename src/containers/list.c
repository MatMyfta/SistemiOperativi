/**
 * \file list.h
 *
 * \brief Generic doubly linked list - implementation
 */

#include "list.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

list *list_create(size_t element_size) {
  list *list = malloc(sizeof(struct list));
  list->head = list->tail = NULL;
  list->size = 0;
  list->element_size = element_size;
  return list;
}

void list_destroy(list *list) {
  list_node *curr;
  for (curr = list->head; curr; curr = curr->next) {
    list_remove_node(list, curr);
  }
  free(list);
}

void *list_back(list *list) { return list->tail->data; }
void list_push_back(list *list, const void *data) {
  list_node *node = malloc(sizeof(struct list_node));
  node->next = NULL;
  node->prev = NULL;
  node->data = malloc(list->element_size);
  memcpy(node->data, data, sizeof(list->element_size));

  ++list->size;
  if (list->tail) {
    // non-empty list
    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
  } else {
    // empty list
    assert(!list->head);
    list->tail = list->head = node;
  }
}
void list_pop_back(list *list) {
  if (list->tail) {
    list_remove_node(list, list->tail);
  }
}

void list_remove(list *list, void *data) {
  list_node *curr;
  for (curr = list->head; curr; curr = curr->next) {
    if (curr->data == data) {
      list_remove_node(list, curr);
      break;
    }
  }
}

void list_remove_node(list *list, list_node *node) {
  free(node->data);
  if (node == list->head) {
    list->head = node->next;
  }
  if (node == list->tail) {
    list->tail = node->prev;
  }
  if (node->prev) {
    node->prev->next = node->next;
  }
  if (node->next) {
    node->next->prev = node->prev;
  }
  free(node);
  --list->size;
}

size_t list_size(list *list) { return list->size; }
