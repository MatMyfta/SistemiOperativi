#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct unitnos_path_node unitnos_path_node;

/*
 * Restituisce un nuovo nodo, se fallisce la creazione restituisce NULL
 */
unitnos_path_node *create_path_node();

/*
 * Dato un nodo, un nome, un path e il riferimento all'index e alle righe,
 * memorizza nei campi del nodo i valori e l'array di file. Rrestituisce 0 se
 * operazione va a buon fine, 1 altrimenti
 */
int fill_path_node(unitnos_path_node *node, char *path);

/*
 * Dealloca un nodo ed il suo contenuto
 */
void remove_node(unitnos_path_node *node);
void print_node(unitnos_path_node *node);

int validate(char *path, char (*(*indexes))[PATH_MAX], int *rows);
char *concat(const char *s1, const char *s2);
