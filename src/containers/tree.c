/**
 * \file tree.c
 *
 * \brief Generic BST - implementation
 */

#include "tree.h"

#include "../utils.h"

#include <assert.h>

/*******************************************************************************
 * Private structures definitions
 *******************************************************************************/
typedef struct unitnos_node unitnos_node;
struct unitnos_node {
  void *value;
  unitnos_node *parent;
  unitnos_node *left;
  unitnos_node *right;
};

/*******************************************************************************
 * Public opaque structures definition
 *******************************************************************************/
struct unitnos_tree {
  unitnos_node *root;
  int size;
  unitnos_compare_func compare_func;
  unitnos_destroy_nodify value_destroy_func;
  void *user_data;
};
/*******************************************************************************
 * Private functions declarations
 *******************************************************************************/
/**
 * Establish connection between the given parent and cur nodes.
 */
static void tree_link(unitnos_tree *tree, unitnos_node *parent,
                      unitnos_node *cur, const void *value);

/**
 * Search for the node containing the given value
 *
 * \retval pointer to node if found
 * \retval NULL if not present
 */
static unitnos_node *tree_search(unitnos_tree *tree, const void *value);
static unitnos_node *tree_min(unitnos_node *node);
static unitnos_node *tree_max(unitnos_node *node);
/**
 * Find the successor of the given node in the tree
 *
 * \retval pointer to successor node if found
 * \retval NULL if not found
 */
static unitnos_node *node_successor(unitnos_tree *tree, unitnos_node *node);
/**
 * Free a node and its value appropriately
 */
static void node_free(unitnos_tree *tree, unitnos_node *node);
/**
 * Destroy subtree rooted at \p node
 */
static void tree_destroy_rec(unitnos_tree *tree, unitnos_node *node);
/**
 * Recursive in-order DFS
 *
 * \retval true if \p func returned true
 * \retval false otherwise
 */
static bool foreach_rec(unitnos_tree *tree, unitnos_node *node,
                        unitnos_tree_transverse_func func, void *user_data);
/*******************************************************************************
 * Public functions implementation
 *******************************************************************************/
unitnos_tree *unitnos_tree_create(unitnos_compare_func compare_func,
                                  unitnos_destroy_nodify value_destroy_func,
                                  void *user_data) {
  unitnos_tree *tree = (unitnos_tree *)unitnos_malloc(sizeof(unitnos_tree));
  tree->root = NULL;
  tree->size = 0;
  tree->compare_func = compare_func;
  tree->value_destroy_func = value_destroy_func;
  tree->user_data = user_data;
  return tree;
}

void unitnos_tree_insert(unitnos_tree *tree, void *value) {
  unitnos_node *parent = NULL;
  unitnos_node *cur = tree->root;

  int compare_res;
  while (cur != NULL && ((compare_res = tree->compare_func(
                              cur->value, value, tree->user_data)) != 0)) {
    parent = cur;
    if (compare_res < 0) {
      cur = cur->right;
    } else {
      cur = cur->left;
    }
  }

  if (cur == NULL) {
    // increment size
    tree->size++;

    unitnos_node *new_node = (unitnos_node *)unitnos_malloc(sizeof(unitnos_node));
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->value = value;
    tree_link(tree, parent, new_node, new_node->value);

    if (parent == NULL) {
      tree->root = new_node;
    }
  }
}
size_t unitnos_tree_size(unitnos_tree *tree) { return tree->size; }

void unitnos_tree_remove(unitnos_tree *tree, const void *value) {
  unitnos_node *tmp = tree_search(tree, value);

  if (tmp == NULL) {
    return;
  }

  --tree->size;
  if (tmp->left == NULL && tmp->right == NULL) {
    // caso 1: il nodo è una foglia
    tree_link(tree, tmp->parent, NULL, value);
    if (tmp->parent == NULL) {
      tree->root = NULL;
    }
    node_free(tree, tmp);
  } else if (tmp->left != NULL && tmp->right == NULL) {
    // caso 2: c'è solo il figlio destro
    tree_link(tree, tmp->parent, tmp->left, value);
    if (tmp->parent == NULL) {
      tree->root = tmp->left;
    }
    node_free(tree, tmp);
  } else if (tmp->left == NULL && tmp->right != NULL) {
    // caso 2: c'è solo il figlio sinistro
    tree_link(tree, tmp->parent, tmp->right, value);
    if (tmp->parent == NULL) {
      tree->root = tmp->right;
    }
    node_free(tree, tmp);
  } else {
    // caso 3: ci sono entrambi i figli
    unitnos_node *successor = node_successor(tree, tmp);
    assert(successor != NULL);
    tree_link(tree, successor->parent, successor->right, successor->value);
    if (tree->value_destroy_func) {
      tree->value_destroy_func(tmp->value, tree->user_data);
    }
    tmp->value = successor->value;
    free(successor);
  }
}

void unitnos_tree_foreach(unitnos_tree *tree, unitnos_tree_transverse_func func,
                          void *user_data) {
  foreach_rec(tree, tree->root, func, user_data);
}

void unitnos_tree_destroy(unitnos_tree *tree) {
  tree_destroy_rec(tree, tree->root);
  free(tree);
}

void *unitnos_tree_lookup(unitnos_tree *tree, const void *value) {
  unitnos_node *node = tree_search(tree, value);

  if (node) {
    return node->value;
  }
  return NULL;
}

bool unitnos_tree_contains(unitnos_tree *tree, const void *value) {
  return tree_search(tree, value) != NULL;
}
void *unitnos_tree_max(unitnos_tree *tree) {
  struct unitnos_node *node = tree_max(tree->root);
  if (node) {
    return node->value;
  }
  return NULL;
}
void *unitnos_tree_min(unitnos_tree *tree) {
  struct unitnos_node *node = tree_min(tree->root);
  if (node) {
    return node->value;
  }
  return NULL;
}
void *unitnos_tree_first(unitnos_tree *tree) {
  if (tree->root) {
    return tree->root->value;
  }
  return NULL;
}
/*******************************************************************************
 * Private functions definitions
 *******************************************************************************/
static void tree_link(unitnos_tree *tree, unitnos_node *parent,
                      unitnos_node *cur, const void *value) {
  if (cur != NULL)
    cur->parent = parent;
  if (parent != NULL) {
    if (tree->compare_func(parent->value, value, tree->user_data) < 0) {
      parent->right = cur;
    } else {
      parent->left = cur;
    }
  }
}

static unitnos_node *tree_search(unitnos_tree *tree, const void *value) {
  unitnos_node *cur = tree->root;

  int compare_res;
  while (cur != NULL && ((compare_res = tree->compare_func(
                              cur->value, value, tree->user_data)) != 0)) {
    if (compare_res < 0) {
      cur = cur->right;
    } else {
      cur = cur->left;
    }
  }

  return cur;
}

static unitnos_node *tree_min(unitnos_node *node) {
  unitnos_node *curr = node;
  unitnos_node *parent = NULL;
  while (curr != NULL) {
    parent = curr;
    curr = curr->left;
  }
  return parent;
}

static unitnos_node *tree_max(unitnos_node *node) {
  unitnos_node *curr = node;
  unitnos_node *parent = NULL;
  while (curr != NULL) {
    parent = curr;
    curr = curr->right;
  }
  return parent;
}

static unitnos_node *node_successor(unitnos_tree *tree, unitnos_node *node) {
  if (node == NULL && tree->root == NULL) {
    return NULL;
  }
  if (node->right != NULL) {
    return tree_min(node->right);
  }
  unitnos_node *tmp = node->parent;
  while (tmp != NULL && node == tmp->right) {
    tmp = node;
    node = node->parent;
  }
  return tmp;
}
static void node_free(unitnos_tree *tree, unitnos_node *node) {
  if (tree->value_destroy_func) {
    tree->value_destroy_func(node->value, tree->user_data);
  }
  free(node);
}
static void tree_destroy_rec(unitnos_tree *tree, unitnos_node *node) {
  if (node != NULL) {
    tree_destroy_rec(tree, node->left);
    tree_destroy_rec(tree, node->right);
    node_free(tree, node);
  }
}
static bool foreach_rec(unitnos_tree *tree, unitnos_node *node,
                        unitnos_tree_transverse_func func, void *user_data) {
  if (!node) {
    return false;
  }

  bool ret = false;
  ret = foreach_rec(tree, node->left, func, user_data);
  if (ret == true) {
    return true;
  }
  ret = func(node->value, user_data);
  if (ret == true) {
    return true;
  }
  ret = foreach_rec(tree, node->right, func, user_data);
  if (ret == true) {
    return true;
  }

  return false;
}
