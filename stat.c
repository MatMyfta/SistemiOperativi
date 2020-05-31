#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
//https://stackoverflow.com/questions/36801833/storing-each-line-of-a-text-file-into-an-array

typedef struct unitnos_statistica unitnos_statistica;
unitnos_statistica *crea_statistica(char *nomefile, int m, int id);
void valuta_carattere(char carattere, unitnos_statistica* statistica);

void recognize(char (*(*indexes))[PATH_MAX], int rows);
char* concat(const char *s1, const char *s2);
int validate(char *path, char (*(*lines))[PATH_MAX], int *rows);

struct unitnos_statistica {
	int lettere;
	int cifre;
	int spazi;
	int punteggiatura;
	int altricaratteri;
	int errore; // indica se statistica contiene dati validi (1) o non è andata a buon fine (!1)
};

unitnos_statistica *crea_statistica (char *nomefile, int m, int id) {

	/* //m è il numero di processi q da generare -> serve per partizionare
	la lettura di un file. id: intero compreso tra 1 e m, indica quale parte deve analizzare del file */
	unitnos_statistica *statistica = malloc(sizeof(unitnos_statistica));
	statistica->lettere=0;
	statistica->cifre=0;
	statistica->spazi=0;
	statistica->punteggiatura=0;
	statistica->altricaratteri=0;
	statistica->errore=1;
	
	// devo accedere a file per leggere -> FILE *fopen(char *name, char *modo)
	FILE *filepointer;
	filepointer=fopen(nomefile, "r");

	if (filepointer==NULL) {
		// Errore nel aprire il file
		statistica->errore=0;
	} else {
		// per leggere da una determinata parte del file uso: int fseek(FILE *stream fd, long int offset, int whence). ritorna 0 se spostamento è avvenuto
		// poi utilizzo ftell per la posizione del contatore
		fseek(filepointer, 0, SEEK_END);
		int dimensionefile = ftell(filepointer);
		if (dimensionefile<=0) {
			statistica->errore=0;
		} else {
			fseek(filepointer, 0, SEEK_SET);
			if (dimensionefile>=id) {	//allora ho qualcosa da leggere e proseguo con la lettura, altrimenti non faccio nulla
				int caratteridaleggere = dimensionefile/m;
				id--;
				fseek(filepointer, (caratteridaleggere * id), SEEK_SET);
				char carattere;
				int i;
				for (i = 0; i < caratteridaleggere; ++i) {
					carattere = fgetc(filepointer);
					if (carattere==EOF) {
						break;
					}
					else {
						valuta_carattere(carattere,statistica);
					}
				}
			}
		}
		fclose(filepointer);
	}

	return statistica;
}

void valuta_carattere (char carattere, unitnos_statistica* statistica) {
	if (((carattere>='a') && (carattere<='z')) || ((carattere>='A') && (carattere<='Z')))
		statistica->lettere++;
	else if (((carattere >= '1') && (carattere <= '9')) || (carattere=='0'))
		statistica->cifre++;
	else if ((carattere=='\t') || (carattere=='\n') || (carattere==' '))
		statistica->spazi++;
	else if ((carattere==',') || (carattere==';') || (carattere=='.') || (carattere==':') || (carattere=='?') || (carattere=='!'))
		statistica->punteggiatura++;
	else
		statistica->altricaratteri++;
}

void stampa_statistica (unitnos_statistica* statistica) {
	if (statistica->errore)
		printf("statistica conta: caratteri: %d, cifre: %d, spazi: %d, punteggiatura: %d, altri caratteri: %d\n", statistica->lettere,
		statistica->cifre, statistica->spazi, statistica->punteggiatura, statistica->altricaratteri);
	else
		printf("statistica invalida o senza dati1\n");
}

char *codifica_statistica (unitnos_statistica* statistica) {
	char *messaggio; // dimensione? limiti capacià messaggio?
	//messaggio = "STAT-%d-%d-%d-%d-%d" // STAT-3-2-35-1-0
	return messaggio;
}

int main(int argc, char *argv[]) {

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
	free(indexes);
	return 0;
}



int validate(char *path, char (*(*indexes))[PATH_MAX], int *rows){ 
	FILE *fp; 
	int rt_value=0;
	
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
	
   	if (!((*indexes) = malloc (((*rows)+1)*sizeof **indexes))) { 
        	printf("Out of memory\n" );
        	exit(-1);
    	} else (*rows)=1;

	while (fgets ((*(indexes))[(*rows)-1], PATH_MAX, fp)) {	  
        	char *p = (*(indexes))[(*rows)-1];                  
        	while(*p != '\n') p++;    
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

void recognize(char (*(*indexes))[PATH_MAX], int rows){
	int x=0;
	while(x<rows){
		printf("[line %d] %s [%ld byte]\n",x,(*(indexes))[x], strlen((*(indexes))[x]));
		x++;
	}
	
	printf("\nTIME FOR SOME STATISTICS\n\n");
	// esempio di uso
	x=0;
	if(x<rows){
		unitnos_statistica* stat1 = crea_statistica ((*(indexes))[x], 2, 1);
		unitnos_statistica* stat2 = crea_statistica ((*(indexes))[x], 2, 2);
		unitnos_statistica* stat3 = crea_statistica ((*(indexes))[x], 1, 1);
		stampa_statistica(stat1);
		stampa_statistica(stat2);
		stampa_statistica(stat3);
	}
	// fine esempio
}


/*  Try to use Lijun function: safe_strcpy_from (logger.c)*/
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2); 
    return result;
}





