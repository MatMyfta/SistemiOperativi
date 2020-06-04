#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

typedef struct unitnos_path_node {
  char *name;
  char (*index)[PATH_MAX];
  int rows;
} unitnos_path_node;

/*
 * Restituisce un nuovo nodo, se fallisce la creazione restituisce NULL
 */
unitnos_path_node* create_path_node ();

/*
 * Dato un nodo, un nome, un path e il riferimento all'index e alle righe, memorizza nei campi del nodo i valori
 * e l'array di file. Rrestituisce 0 se operazione va a buon fine, 1 altrimenti
 */
int fill_path_node (unitnos_path_node* node, char *path);

/*
 * Dealloca un nodo ed il suo contenuto
 */
void remove_node (unitnos_path_node* node);

/*
 * Dati due nodi li confronta in base al nome
 */
int compare (unitnos_path_node* node1, unitnos_path_node* node2);

int validate(char *path, char (*(*indexes))[PATH_MAX], int *rows);
char* concat(const char *s1, const char *s2);
