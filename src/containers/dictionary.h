#include <stdio.h>
#include <stdlib.h>

typedef struct unitnos_dictionary unitnos_dictionary;
typedef struct unitnos_dictionary_node unitnos_dictionary_node;

/**
 * Restituisce un nuovo dizionario
 *
 * Ciascun dizionario ha le sue funzioni per il confronto e la liberazione della
 * memoria
 */
unitnos_dictionary *unitnos_dictionary_create(int (*compare)(void *key1, void *key2),
                                  int (*free_mem_key)(void *key),
                                  int (*free_mem_value)(void *value));

/**
 * Aggiunge un nodo con la chiave e il valore indicato. Se un nodo con la stessa chaive è già presente lo elimina prima
 */
void unitnos_dictionary_add_node(unitnos_dictionary *dictionary, void *key, void *value);

/**
 * Restituisce un nodo con la chiave indicata
 *
 * \retval NULL se non lo trova
 */
unitnos_dictionary_node *unitnos_dictionary_search(unitnos_dictionary *dictionary, void *key);

/**
 * Rimuove il nodo che coincide con la chiave indicato
 */
void unitnos_dictionary_remove(unitnos_dictionary *dictionary, void *key);

/**
 * Rimuovono ricorsivamente tutti i nodi a partire da un nodo specificato
 * [destroy] o dalla root [destroy_all]
 */
void unitnos_dictionary_destroy(unitnos_dictionary *dictionary, unitnos_dictionary_node *node);
void unitnos_dictionary_destroy_all(unitnos_dictionary *dictionary);