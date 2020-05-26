#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void recognize(char **indexes, int row);
char* concat(const char *s1, const char *s2);
int validate(char *path, char **read, int *row);

int main(int argc, char *argv[]) {  // IN DOCKER NON FUNZIONA dunno why

	if(argc!=2) {
		printf ("Usage execname <path>\n");
		return -1;
	}

	
	int row = 0; 
	char **indexes=(char **)malloc(((row)) * sizeof(char *)); // alloco lo spazio per contenere #row puntatori a carattere

	printf("[_define] (tot: %d, index: 0) ROOT_index: %d - ROOT_dimension: %d byte\n",row+1, indexes,sizeof(indexes));

	if(validate(argv[1], indexes, &row)) { // se il valore di ritorno è 1 (ritorno da bash>find valido
		recognize(indexes, row); // vado a fare statistiche sulla matrice appena salvata
	} else {
		printf("<path> not valid\n");
		return -2;
	}

	return 0;
}




/*
* FUNZIONE PER LA LETTURA E SALVATAGGIO DEI VALORI RITORNATI DA BASH>FIND
* 
* Il problema sta nel fatto che a determinati punti del salvataggio nella matrice ci sono errori legati a realloc (a allocazioni in generale)
* In particolare non capisco il motivo per cui riallocando lo spazio nel momento in cui devo contenere una nuova riga (codice tra spazi ampi)	
* l'indirizzo dell'oggetto cambia sebbene sia utilizzato realloc. Commentando quella riga, per assurdo, il ciclo funziona fino a un massimo di 20 righe. dafaq
*/



int validate(char *path, char **read, int *row){ 
	FILE *fp; int ret=0; 
	
	char *command = concat("/bin/find ", path); // Not able to intercept stderr -> verifico presenza di stringa di errore
	
	command = concat(command," -type f 2>&1"); //   stderr goes on stdout
	fp = popen(command, "r"); 
	free(command);

 	if (fp == NULL) { // in quali casi potrebbe tornare false?
		printf("Failed to run command\n" );
		exit(-1);
	}
	
   	int n = 0,chars=0,c;

	while ((c = fgetc(fp)) != EOF) { // *row -> indice, (*row)+1 -> dimensione
		if(n==0){
        		read[*row] = (char *)malloc(chars * sizeof(char)); // preparo il puntatore read[0] a ricevere dei caratteri
		}
		chars++; // viene aumentato il numero di caratteri per la riga *row attuale
		if(c!='\n') { // se è un carattere di stringa 			
			n++; // n viene aumentato a ogni lettura diversa da newlne (inutilizzato in seguito)			
			read[*row]=(char *)realloc(read[*row], chars*sizeof(char)); // rialloco lo spazio puntato da read[0] per contenete #chars caratteri
        		read[*row][chars-1] = (char) c; //inserisco il carattere a pos chars-1
		} else { // se la stringa è finita...					
			if(n!=0) { // ...e non sono all'inizio dello stream
				if(strstr(read[*row], "/bin/find:") != NULL) { // se la riga precedente contiene "/bin/find" (statement di errore per find in bash)
					free(read); // dealloco lo spazio 
					ret=0; // setto il valore di ritorno per errore -> path invalido/non riconosciuto
					break; //esco
				} else ret = 1; // altrimenti significa che la find ha ritorno valido e continuo a leggere lo stream (settando il vaore di ritorno a 1)
				read[*row][chars] = '\0'; //termino la riga attuale
				chars=0; // resetto la variabile che tiene conto dei caratteri per riga
			} 
			(*row)++; // ...ho trovato il newline quindi predispongo la matrice a contenete una nuova riga, e aumento *row di uno




	

			read = (char **)realloc(read,((*row)+1) * sizeof(char *)); // rialloco lo spazio per conentere una riga in più

			
	



			printf("[new_row] (tot: %d, index: %d) ROOT_index: %d - ROOT_dimension: %d byte\n",((*row)+1),(*row), read, ((*row)+1) * sizeof(char *));
			read[*row] = (char *)malloc(chars * sizeof(char)); // predispongo la nuova riga read[*row] a contenere #chars caratteri
		}
    	}

	free(read[*row]);  // ultimo newline produce una nuova riga nella matrice che però non viene mai inserita a causa dell'EOF del ciclo successivo, quindi 
	//read[*row]=NULL; 
	(*row)--; // riporto il numero all'ultimo indice utilizzato

 	pclose(fp);
	return ret;
}


void recognize(char **indexes, int row){
	int x=0;
	while(x<=(row)){
		printf("%s [%ld byte]\n",indexes[x], strlen(indexes[x]));
		x++;
	}
	printf("TIME FOR SOME STATISTICS\n");
}




/*  Try to use Lijun function: safe_strcpy_from (logger.c)*/
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2); 
    return result;
}






