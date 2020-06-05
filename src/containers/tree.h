/**
 * \file tree.h
 *
 * \brief Generic BST - interface
 *
 * It can be used to build more advanced/specific data structures.
 */
#ifndef UNITNOS_TREE_H_
#define UNITNOS_TREE_H_

#include "base.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct unitnos_tree unitnos_tree;
typedef struct unitnos_node unitnos_node;

/**
 * Restituisce un nuovo albero
 *
 * Ciascun albero ha le sue funzioni per il confronto e la liberazione della
 * memoria
 *
 * \param [in] user_data optional user data pased to compare_func and
 * value_destroy_func
 */
unitnos_tree *unitnos_tree_create(unitnos_compare_func compare_func,
                                  unitnos_destroy_nodify value_destroy_func,
                                  void *user_data);

/**
 * Aggiunge un nodo con il valore indicato
 */
void unitnos_tree_add_node(unitnos_tree *tree, void *value);

/**
 * Restituisce un nodo col valore indicato
 *
 * \retval NULL se non lo trova
 */
unitnos_node *unitnos_tree_search(unitnos_tree *tree, void *value);

/**
 * Rimuove il nodo che coincide col valore indicato
 */
void unitnos_tree_remove(unitnos_tree *tree, void *value);

/**
 * Rimuovono ricorsivamente tutti i nodi a partire da un nodo specificato
 * [destroy] o dalla root [destroy_all]
 */
void unitnos_tree_destroy(unitnos_tree *tree, unitnos_node *node);
void unitnos_tree_destroy_all(unitnos_tree *tree);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_TREE_H_ */
