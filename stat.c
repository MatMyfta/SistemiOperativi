#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
//https://stackoverflow.com/questions/36801833/storing-each-line-of-a-text-file-into-an-array

void recognize(char (*(*indexes))[PATH_MAX], int rows);
char* concat(const char *s1, const char *s2);
int validate(char *path, char (*(*lines))[PATH_MAX], int *rows);


int main(int argc, char *argv[]) {  // IN DOCKER NON FUNZIONA dunno why

	if(argc!=2) {
		printf ("Usage execname <path>\n");
		return -1;
	}
	int rows = 0; 
	char (*indexes)[PATH_MAX] = NULL;  
	if(validate(argv[1], &indexes, &rows)) {
		recognize(&indexes, rows); // reference or value ?
	} else {
		printf("<path> not valid\n");
		return -2;
	}

	return 0;
}



int validate(char *path, char (*(*lines))[PATH_MAX], int *rows){ 
	FILE *fp; 
	int rt_value=0;
	
	char *command = concat("/bin/find ", path); // Not able to intercept stderr
	command = concat(command," -type f 2>&1"); //   stderr goes on stdout
	fp = popen(command, "r"); 
	free(command);

	if (!fp) {  
        	printf("Failed to run command\n" );
        	exit(-1);
    	}
	
   	if (!((*lines) = malloc (((*rows)+1)*sizeof **lines))) { 
        	printf("Out of memory\n" );
        	exit(-1);
    	} else (*rows)=1;

	while (fgets ((*(lines))[(*rows)-1], PATH_MAX, fp)) {	
		if((*rows)==1 && strstr((*(lines))[(*rows)-1], "/bin/find:") != NULL) { 
			free((*lines));
			rt_value=0;
			break;
		} else rt_value = 1;   

        	char *p = (*(lines))[(*rows)-1];                  
        	while(*p != '\n') p++;    
        	*p = 0;
  		void *tmp = realloc ((*lines), ((*rows)+1) * sizeof **lines);
   		if (!tmp) {     
       			printf ("Out of memory\n");
        		break;
    		}
    		(*lines) = tmp;
    		(*rows)++;
 	}
	(*rows)--;
	//for (int i = 0; i < (*rows)-1; i++) printf (" line[%2d] : '%s'\n", i + 1, (*(lines))[i]);
 	fclose (fp);
  

    	return rt_value;
}

void recognize(char (*(*indexes))[PATH_MAX], int rows){
	int x=0;
	while(x<rows){
		printf("[line %d] %s [%ld byte]\n",x,(*(indexes))[x], strlen((*(indexes))[x]));
		x++;
	}
	
	printf("\nTIME FOR SOME STATISTICS\n\n");
}


/*  Try to use Lijun function: safe_strcpy_from (logger.c)*/
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2); 
    return result;
}
