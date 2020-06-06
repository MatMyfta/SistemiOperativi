/**
 * \file dictionary.c
 *
 * \brief Generic ordered dictionary - implementation
 */
#include "dictionary.h"

#include "tree.h"

/*******************************************************************************
 * Private structures definitions
 *******************************************************************************/
struct unitnos_dictionary_node {
  void *key;
  void *value;
};

/*******************************************************************************
 * Public opaque structures definition
 *******************************************************************************/
struct unitnos_dictionary {
  /**
   * The dictionary is based on the BST
   */
  unitnos_tree *tree;
  unitnos_compare_func compare_func;
  unitnos_destroy_nodify key_destroy_func;
  unitnos_destroy_nodify value_destroy_func;
  void *user_data;
};

/*******************************************************************************
 * Private functions declarations
 *******************************************************************************/
static int node_compare(const void *lhs, const void *rhs, void *dictionary);
static void node_destroy(void *node, void *dictionary);

/*******************************************************************************
 * Public functions implementation
 *******************************************************************************/
unitnos_dictionary *unitnos_dictionary_create(
    unitnos_compare_func compare_func, unitnos_destroy_nodify key_destroy_func,
    unitnos_destroy_nodify value_destroy_func, void *user_data) {
  unitnos_dictionary *dict =
      (unitnos_dictionary *)malloc(sizeof(unitnos_dictionary));
  dict->tree = unitnos_tree_create(node_compare, node_destroy, dict);
  dict->compare_func = compare_func;
  dict->key_destroy_func = key_destroy_func;
  dict->value_destroy_func = value_destroy_func;
  dict->user_data = user_data;
  return dict;
}
void unitnos_dictionary_destroy(unitnos_dictionary *dict) {
  unitnos_tree_destroy(dict->tree);
  free(dict);
}
void unitnos_dictionary_insert(unitnos_dictionary *dictionary, void *key,
                               void *value) {
  struct unitnos_dictionary_node *node =
      (struct unitnos_dictionary_node *)malloc(
          sizeof(struct unitnos_dictionary_node));
  node->key = key;
  node->value = value;
  unitnos_tree_insert(dictionary->tree, node);
}
void *unitnos_dictionary_lookup(unitnos_dictionary *dictionary,
                                const void *key) {
  struct unitnos_dictionary_node node;
  node.key = (void *)key;
  void *n = unitnos_tree_lookup(dictionary->tree, &node);
  if (n) {
    return ((struct unitnos_dictionary_node *)n)->value;
  }
  return NULL;
}
bool unitnos_dictionary_contains(unitnos_dictionary *dictionary, const void *key) {
  struct unitnos_dictionary_node node;
  node.key = (void *)key;
  return unitnos_tree_contains(dictionary->tree, &node);
}
void unitnos_dictionary_remove(unitnos_dictionary *dictionary, const void *key) {
  struct unitnos_dictionary_node node;
  node.key = (void *)key;
  unitnos_tree_remove(dictionary->tree, &node);
}
size_t unitnos_dictionary_size(unitnos_dictionary *dictionary) {
  return unitnos_tree_size(dictionary->tree);
}
void unitnos_dictionary_foreach(unitnos_dictionary *dictionary,
                                unitnos_dictionary_transverse_func func,
                                void *user_data) {
  // GNU C extension
  bool transverse (void *val, void *user_data) {
    struct unitnos_dictionary_node *node = (struct unitnos_dictionary_node *)val;
    return func(node->key, node->value, user_data);
  }
  unitnos_tree_foreach(dictionary->tree, transverse, user_data);
}

/*******************************************************************************
 * Private functions implementation
 *******************************************************************************/
static int node_compare(const void *lhs, const void *rhs, void *dictionary) {
  unitnos_dictionary *dict = (unitnos_dictionary *)dictionary;
  return dict->compare_func(((struct unitnos_dictionary_node *)lhs)->key,
                            ((struct unitnos_dictionary_node *)rhs)->key,
                            dict->user_data);
}
static void node_destroy(void *val, void *dictionary) {
  unitnos_dictionary *dict = (unitnos_dictionary *)dictionary;
  struct unitnos_dictionary_node *node = (struct unitnos_dictionary_node *)val;
  if (dict->key_destroy_func) {
    dict->key_destroy_func(node->key, dict->user_data);
  }
  if (dict->value_destroy_func) {
    dict->value_destroy_func(node->value, dict->user_data);
  }
  free(node);
}
