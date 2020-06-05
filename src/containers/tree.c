/**
 * \file tree.c
 *
 * \brief Generic BST - implementation
 */

#include "tree.h"

struct unitnos_tree {
  unitnos_node *root;
  int size;
  unitnos_compare_func compare_func;
  unitnos_destroy_nodify value_destroy_func;
  void *user_data;
};

struct unitnos_node {
  void *value;
  struct unitnos_node *parent;
  struct unitnos_node *left;
  struct unitnos_node *right;
};

unitnos_tree *unitnos_tree_create(unitnos_compare_func compare_func,
                                  unitnos_destroy_nodify value_destroy_func,
                                  void *user_data) {
  unitnos_tree *tree = (unitnos_tree *)malloc(sizeof(unitnos_tree));
  tree->size = 0;
  tree->compare_func = compare_func;
  tree->value_destroy_func = value_destroy_func;
  tree->user_data = user_data;
  return tree;
}

static void unitnos_tree_add_node_wrap(unitnos_tree *tree, unitnos_node *node,
                                       unitnos_node *tmp) {
  if (node == NULL)
    node = tmp;
  else { // BISOGNEREBBE ANCHE AUMENTARE LA DIMENSIONE DELL'ALBERO SIZE
    tmp->parent = node;
    if (tree->compare_func(node, tmp, tree->user_data) > 0)
      unitnos_tree_add_node_wrap(tree, node->right, tmp);
    else
      unitnos_tree_add_node_wrap(tree, node->left, tmp);
  }
}
void unitnos_tree_add_node(unitnos_tree *tree, void *value) {
  unitnos_node *node_tmp;
  node_tmp->value = value;
  node_tmp->left = NULL;
  node_tmp->right = NULL;
  node_tmp->parent = NULL;

  unitnos_tree_add_node_wrap(tree, tree->root, node_tmp);
}

static unitnos_node *unitnos_tree_search_wrap(unitnos_tree *tree,
                                              unitnos_node *node, void *value) {
  if (tree->compare_func(node->value, value, tree->user_data) == 0 ||
      node == NULL)
    return node;
  if (tree->compare_func(node->value, value, tree->user_data) > 0)
    return unitnos_tree_search_wrap(tree, node->right, value);
  return unitnos_tree_search_wrap(tree, node->left, value);
}

unitnos_node *unitnos_tree_search(unitnos_tree *tree, void *value) {
  return unitnos_tree_search_wrap(tree, tree->root, value);
}

static unitnos_node *unitnos_tree_min(unitnos_node *node) {
  unitnos_node *tmp = node;
  while (tmp->left != NULL)
    tmp = tmp->left;
  return tmp;
}

static unitnos_node *unitnos_node_successor(unitnos_tree *tree,
                                            unitnos_node *node) {
  if (node == NULL && tree->root == NULL)
    return NULL;
  if (node->right != NULL)
    return unitnos_tree_min(node->right);
  unitnos_node *tmp = node->parent;
  while (tmp != NULL && node == tmp->right) {
    tmp = node;
    node = node->parent;
  }
  return tmp;
}

static void unitnos_tree_link(unitnos_tree *tree, unitnos_node *node,
                              unitnos_node *u) {
  if (u != NULL)
    u->parent = node;
  if (node != NULL) {
    if (tree->compare_func(node, u, tree->user_data) < 0)
      node->left = u;
    else
      node->right = u;
  }
}

static void unitnos_node_free(unitnos_tree *tree, unitnos_node *node) {
  tree->value_destroy_func(node->value, tree->user_data);
  free(node);
}

void unitnos_tree_remove(unitnos_tree *tree, void *value) {
  unitnos_node *tmp = unitnos_tree_search(tree, value);
  if (tmp == NULL) {
    fprintf(stderr, "ERROR: no such node in the tree\n");
  } else {
    if (tmp->left == NULL && tmp->right == NULL) {
      // caso 1: il nodo è una foglia
      unitnos_tree_link(tree, tmp->parent, NULL);
      unitnos_node_free(tree, tmp);
    } else if (tmp->left != NULL && tmp->right == NULL) {
      // caso 2: c'è solo il figlio destro
      unitnos_tree_link(tree, tmp->parent, tmp->left);
      if (tmp->parent == NULL)
        tree->root = tmp->left;
    } else if (tmp->left == NULL && tmp->right != NULL) {
      // caso 2: c'è solo il figlio sinistro
      unitnos_tree_link(tree, tmp->parent, tmp->right);
      if (tmp->parent == NULL)
        tree->root = tmp->right;
    } else {
      // caso 3: ci sono entrambi i figli
      unitnos_node *successor = unitnos_node_successor(tree, tmp);
      unitnos_tree_link(tree, successor->parent, successor->right);
      tmp->value = successor->value;
      unitnos_node_free(tree, successor);
    }
  }
}

void unitnos_tree_destroy(unitnos_tree *tree, unitnos_node *node) {
  if (node != NULL) {
    unitnos_tree_destroy(tree, node->left);
    unitnos_tree_destroy(tree, node->right);
    unitnos_node_free(tree, node);
  }
}
void unitnos_tree_destroy_all(unitnos_tree *tree) {
  unitnos_tree_destroy(tree, tree->root);
}
