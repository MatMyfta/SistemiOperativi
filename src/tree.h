/*
	restituisce un nuovo albero
	ciascun albero ha le sue funzioni per il confronto e la liberazione della memoria
*/
unitnos_tree* unitnos_tree_create( void ( *compare )( void *v1, void *v2 )
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
