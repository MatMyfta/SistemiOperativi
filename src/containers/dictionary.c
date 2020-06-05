#include "dictionary.h"

struct unitnos_dictionary {
  unitnos_dictionary_node *root;
  int size;
  /**
   * compare deve restituire 1 se v2 è maggiore, 0 se uguale o -1 se è minore. Dovrà basarsi su chiave
   */
  int (*compare)(void *key1, void *key2);
  /**
   * ciascun tipo ha due funzioni propria per la liberazione della memoria (una per key e una per value) deve
   * restituire 1 se va a buon fine, 0 altrimenti
   */
  int (*free_mem_value)(void *value);
  int (*free_mem_key)(void *key);
};

struct unitnos_dictionary_node {
  void *key;
  void *value;
  struct unitnos_dictionary_node *parent;
  struct unitnos_dictionary_node *left;
  struct unitnos_dictionary_node *right;
};

unitnos_dictionary *unitnos_dictionary_create(int (*compare)(void *key1, void *key2),
                                  int (*free_mem_value)(void *value),
                                  int (*free_mem_key)(void *key)) {
  unitnos_dictionary *dictionary = (unitnos_dictionary *)malloc(sizeof(unitnos_dictionary));
  dictionary->size = 0;
  dictionary->compare = compare;
  dictionary->free_mem_value = free_mem_value;
  dictionary->free_mem_key = free_mem_key;
  return dictionary;
}

static void unitnos_dictionary_add_node_wrap(unitnos_dictionary *dictionary, unitnos_dictionary_node *node,
                                       unitnos_dictionary_node *tmp) {
  if (node == NULL)
    node = tmp;
  else {
    tmp->parent = node;
    if (dictionary->compare(node->key, tmp->key) > 0)
      unitnos_dictionary_add_node_wrap(dictionary, node->right, tmp);
    else
      unitnos_dictionary_add_node_wrap(dictionary, node->left, tmp);
  }
}
void unitnos_dictionary_add_node(unitnos_dictionary *dictionary, void* key, void *value) {
  unitnos_dictionary_node *node_tmp;
  node_tmp->key = key;
  node_tmp->value = value;
  node_tmp->left = NULL;
  node_tmp->right = NULL;
  node_tmp->parent = NULL;

  unitnos_dictionary_add_node_wrap(dictionary, dictionary->root, node_tmp);
}

static unitnos_dictionary_node *unitnos_dictionary_search_wrap(unitnos_dictionary *dictionary,
                                              unitnos_dictionary_node *node, void *key) {
  if (dictionary->compare(node->key, key) == 0 || node == NULL)
    return node;
  if (dictionary->compare(node->key, key) > 0)
    return unitnos_dictionary_search_wrap(dictionary, node->right, key);
  return unitnos_dictionary_search_wrap(dictionary, node->left, key);
}

unitnos_dictionary_node *unitnos_dictionary_search(unitnos_dictionary *dictionary, void *key) {
  return unitnos_dictionary_search_wrap(dictionary, dictionary->root, key);
}

static unitnos_dictionary_node *unitnos_dictionary_min(unitnos_dictionary_node *node) {
  unitnos_dictionary_node *tmp = node;
  while (tmp->left != NULL)
    tmp = tmp->left;
  return tmp;
}

static unitnos_dictionary_node *unitnos_dictionary_node_successor(unitnos_dictionary *dictionary,
                                            unitnos_dictionary_node *node) {
  if (node == NULL && dictionary->root == NULL)
    return NULL;
  if (node->right != NULL)
    return unitnos_dictionary_min(node->right);
  unitnos_dictionary_node *tmp = node->parent;
  while (tmp != NULL && node == tmp->right) {
    tmp = node;
    node = node->parent;
  }
  return tmp;
}

static void unitnos_dictionary_link(unitnos_dictionary *dictionary, unitnos_dictionary_node *node,
                              unitnos_dictionary_node *u) {
  if (u != NULL)
    u->parent = node;
  if (node != NULL) {
    if (dictionary->compare(node->key, u->key) < 0)
      node->left = u;
    else
      node->right = u;
  }
}

static void unitnos_dictionary_node_free(unitnos_dictionary *dictionary, unitnos_dictionary_node *node) {
  dictionary->free_mem_key(node->key);
  dictionary->free_mem_value(node->value);
  free(node);
}

void unitnos_dictionary_remove(unitnos_dictionary *dictionary, void *key) {
  unitnos_dictionary_node *tmp = unitnos_dictionary_search(dictionary, key);
  if (tmp == NULL) {
    fprintf(stderr, "ERROR: no such node in the dictionary\n");
  } else {
    if (tmp->left == NULL && tmp->right == NULL) {
      // caso 1: il nodo è una foglia
      unitnos_dictionary_link(dictionary, tmp->parent, NULL);
      unitnos_dictionary_node_free(dictionary, tmp);
    } else if (tmp->left != NULL && tmp->right == NULL) {
      // caso 2: c'è solo il figlio destro
      unitnos_dictionary_link(dictionary, tmp->parent, tmp->left);
      if (tmp->parent == NULL)
        dictionary->root = tmp->left;
    } else if (tmp->left == NULL && tmp->right != NULL) {
      // caso 2: c'è solo il figlio sinistro
      unitnos_dictionary_link(dictionary, tmp->parent, tmp->right);
      if (tmp->parent == NULL)
        dictionary->root = tmp->right;
    } else {
      // caso 3: ci sono entrambi i figli
      unitnos_dictionary_node *successor = unitnos_dictionary_node_successor(dictionary, tmp);
      unitnos_dictionary_link(dictionary, successor->parent, successor->right);
      tmp->key = successor->key;
      tmp->value = successor->value;
      unitnos_dictionary_node_free(dictionary, successor);
    }
  }
}

void unitnos_dictionary_destroy(unitnos_dictionary *dictionary, unitnos_dictionary_node *node) {
  if (node != NULL) {
    unitnos_dictionary_destroy(dictionary, node->left);
    unitnos_dictionary_destroy(dictionary, node->right);
    unitnos_dictionary_node_free(dictionary, node);
  }
}
void unitnos_dictionary_destroy_all(unitnos_dictionary *dictionary) {
  unitnos_dictionary_destroy(dictionary, dictionary->root);
}