/**
 * \file list.h
 *
 * \brief Generic doubly linked list - implementation
 */

#include "list.h"

#include "../utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct unitnos_list_node {
  unitnos_list_node *prev;
  unitnos_list_node *next;
  void *data;
};

struct unitnos_list {
  unitnos_list_node *head;
  unitnos_list_node *tail;
  size_t size;
  unitnos_destroy_nodify value_destroy_func;
  void *user_data;
};

/*******************************************************************************
 * Private functions declarations
 *******************************************************************************/
static void unitnos_list_remove_node(unitnos_list *list,
                                     unitnos_list_node *node);

/*******************************************************************************
 * Public functions implementation
 *******************************************************************************/
unitnos_list *unitnos_list_create(unitnos_destroy_nodify value_destroy_func,
                                  void *user_data) {
  unitnos_list *list = unitnos_malloc(sizeof(struct unitnos_list));
  list->head = list->tail = NULL;
  list->size = 0;
  list->value_destroy_func = value_destroy_func;
  list->user_data = user_data;
  return list;
}

void unitnos_list_destroy(unitnos_list *list) {
  unitnos_list_node *curr;
  for (curr = list->head; curr; curr = curr->next) {
    unitnos_list_remove_node(list, curr);
  }
  free(list);
}

void *unitnos_list_back(unitnos_list *list) { return list->tail->data; }
void unitnos_list_push_back(unitnos_list *list, void *data) {
  unitnos_list_node *node = unitnos_malloc(sizeof(struct unitnos_list_node));
  node->next = NULL;
  node->prev = NULL;
  node->data = data;

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
void unitnos_list_pop_back(unitnos_list *list) {
  if (list->tail) {
    unitnos_list_remove_node(list, list->tail);
  }
}

void unitnos_list_remove(unitnos_list *list, const void *data) {
  unitnos_list_node *curr = list->head;
  for (curr = list->head; curr; curr = curr->next) {
    if (curr->data == data) {
      unitnos_list_remove_node(list, curr);
      break;
    }
  }
}

size_t unitnos_list_size(unitnos_list *list) { return list->size; }

void unitnos_list_foreach(unitnos_list *list, unitnos_list_transverse_func func,
                          void *user_data) {
  unitnos_list_node *curr = list->head;
  for (curr = list->head; curr; curr = curr->next) {
    if (func(curr->data, user_data)) {
      break;
    }
  }
}

/*******************************************************************************
 * Private functions implementation
 *******************************************************************************/
static void unitnos_list_remove_node(unitnos_list *list,
                                     unitnos_list_node *node) {
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
  if (list->value_destroy_func) {
    list->value_destroy_func(node->data, list->user_data);
  }
  free(node);
  --list->size;
}
