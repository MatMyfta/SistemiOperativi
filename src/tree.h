#ifndef UNITNOS_TREE_H_
#define UNITNOS_TREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

typedef struct unitnos_node {
	void* value;
	struct unitnos_node* parent;
	struct unitnos_node* left;
	struct unitnos_node* right;
} unitnos_node;

typedef struct unitnos_tree {
	unitnos_node* root;
	int size;

	// compare deve restituire 1 se v2 è maggiore, 0 se uguale o -1 se è minore
	int ( *compare )( void *v1, void *v2 );
	// ciascun tipo ha una funzione propria per la liberazione della memoria
	// deve restituire 1 se va a buon fine, 0 altrimenti
	int( *free_mem )( void *value );
} unitnos_tree;

/*
	restituisce un nuovo albero
	ciascun albero ha le sue funzioni per il confronto e la liberazione della memoria
*/
unitnos_tree* unitnos_tree_create(	int ( *compare )( void *v1, void *v2 ),
									int ( *free_mem )( void *value ) );

/*
	aggiunge un nodo con il valore indicato
*/
void unitnos_tree_add_node( unitnos_tree *tree, void *value );

/*
	restituisce un nodo col valore indicato
	NULL se non lo trova
*/
unitnos_node* unitnos_tree_search( unitnos_tree *tree, void *value );

/*
	rimuove il nodo che coincide col valore indicato
*/
void unitnos_tree_remove( unitnos_tree *tree, void *value );

/*
	rimuovono ricorsivamente tutti i nodi
	a partire da un nodo specificato [destroy] o dalla root [destroy_all]
*/
void unitnos_tree_destroy( unitnos_tree *tree, unitnos_node *node );
void unitnos_tree_destroy_all( unitnos_tree *tree );

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_TREE_H_ */
