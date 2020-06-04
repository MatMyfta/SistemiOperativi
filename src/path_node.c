#include "path_node.h"

unitnos_path_node* create_path_node () {
	return malloc(sizeof(unitnos_path_node));
}

int fill_path_node (unitnos_path_node* node, char* path) {
	int return_value = 0;
	if (node!=NULL) {
		if (node->name = malloc(strlen(path)+1)) {
			strcpy(node->name, path);
			node->rows=0;
			if (node->index = malloc (((node->rows)+1)*sizeof(node->index))) {
				if (!validate(node->name, &node->index, &node->rows))	// quando arrivo qui ho allocato tutto ciò che mi serve e posso provare a convertire i path
					return_value = 1;
			} else
				return_value = 1;
		} else
			return_value = 1; 
	} else
		return_value = 1;
	return return_value;
}

void remove_node (unitnos_path_node* node) {
	int i;
	node->rows=0;
	free(node->index);
	free(node->name);
	free(node);

}

void print_node (unitnos_path_node* node) {
	int x=0;
	while(x<node->rows){
		printf("[path: %s] - [index %d:%s]\n", node->name, x, node->index[x]);
		x++;
	}
}

int compare (unitnos_path_node* node1, unitnos_path_node* node2) {
	return strcmp(node1->name, node2->name);
}

// PROVA MAIN
int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("usage: execname <path>");
		exit(1);
	} else {
		unitnos_path_node* nodo = create_path_node();
		char* path = malloc(PATH_MAX * sizeof(char));
		strcpy(path, argv[1]);
		fill_path_node(nodo, path);
		print_node(nodo);
		remove_node(nodo);
		free(path);
	}
}

int validate(char *path, char (*(*indexes))[PATH_MAX], int *rows) { 
	FILE *fp; 
	int rt_value=0;
	*rows=1;
	
	/*
	* In docker il comando find si trova in /usr/bin/find invece che in /bin/find
    * Comportamenti analoghi e controllati per entrambi gli ambienti.
	*/

	char *command = concat("/usr/bin/find ", path); // Not able to intercept stderr
	command = concat(command," -type f 2>&1"); //   stderr goes on stdout
	fp = popen(command, "r"); 
	free(command);

	if (!fp) {  
        	printf("Failed to run command\n" );
        	exit(-1);
    	}	
	while (fgets ((*(indexes))[(*rows)-1], PATH_MAX, fp)) {	  
        char *p = (*(indexes))[(*rows)-1];                  
        while(*p != '\n')
        	p++;    
        *p = 0;
  		void *tmp = realloc ((*indexes), ((*rows)+1) * sizeof **indexes);
   		if (!tmp) {     
       			printf ("Out of memory\n");
        		break;
    		}
    	(*indexes) = tmp;
    	(*rows)++;
 	}
	(*rows)--;
	//for (int i = 0; i < (*rows)-1; i++) printf (" line[%2d] : '%s'\n", i + 1, (*(indexes))[i]);
 	fclose (fp);

	if((*rows)==1 && strstr((*(indexes))[(*rows)-1], "/bin/find:") != NULL) { 
			free((*indexes));
			rt_value=0;
		} else rt_value = 1;  

    	return rt_value;
}

/*  Try to use Lijun function: safe_strcpy_from (logger.c)*/
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2); 
    return result;
}