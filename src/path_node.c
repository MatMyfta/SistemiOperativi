#include "path_node.h"

unitnos_path_node* create_path_node () {
	return malloc(sizeof(unitnos_path_node));
}

int fill_path_node (unitnos_path_node* node, char *path) {
	int return_value = 0;
	if (node!=NULL) {
		node->name=path;
		node->rows=0;
		node->index = NULL;
		printf("prova\n");
	} else
		return_value = 1;
	return return_value;
}

void remove_node (unitnos_path_node *node) {
	int i;
	node->rows=0;
	printf("%s\n", node->name);
	free(node);
}

// PROVA MAIN
int main() {
	unitnos_path_node* nodo1 = create_path_node();
	unitnos_path_node* nodo2 = create_path_node();
	char* path1 = malloc(PATH_MAX * sizeof(char));
	strcpy(path1, "main");
	fill_path_node(nodo1, path1);
	strcpy(path1, "main2");
	fill_path_node(nodo2, path1);
	remove_node(nodo1);
	remove_node(nodo2);
	free(path1);
}
