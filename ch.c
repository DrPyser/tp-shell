/* ch.c --- Un shell pour les hélvètes.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Pour les tests.
#define memcheck(x) if(x == NULL) {\
                        printf("Mémoire epuisée.\n");\
                        return NULL;\
                    }

/**
 *Writes the data from the stream "from" to a file named "filename".
 *If "filename" designates an already existing file, the file is overwritten.
 *Otherwise, a new file is created.
*/
int outputToFile(FILE* from, char* filename){
  FILE* output = fopen(filename, "w");
  int c;
  int ret_code;
  while((c = fgetc(from)) != EOF){
    ret_code = fputc(c,output);
    if(ret_code == EOF){
      fprintf(stderr,"failure to write character %c in file %s", c, filename);
      fclose(output);
      return EXIT_FAILURE;
    }
  }

  fflush(output);
  fclose(output);

  return EXIT_SUCCESS;
}

int appendToFile(FILE* from, char* filename){
  FILE* output = fopen(filename, "a");
  int c;
  int ret_code;
  while((c = fgetc(from)) != EOF){
    ret_code = fputc(c,output);
    if(ret_code == EOF){
      fprintf(stderr,"failure to write character %c in file %s", c, filename);
      fclose(output);
      return EXIT_FAILURE;
    }
  }

  fflush(output);
  fclose(output);

  return EXIT_SUCCESS;
}

int pipe(FILE* from, FILE* to){
  int c;
  int ret_code;
  while((c = fgetc(from)) != EOF){
    ret_code = fputc(c,to);
    if(ret_code == EOF){
      fprintf(stderr,"failure to write character %c to output stream", c);
      return EXIT_FAILURE;
    }
  }
  fflush(from);
  return EXIT_SUCCESS;
}

int execProgram(char* pgmName, char** args)
{
  return 0;
}

//int inputFrom(FILE* to, FILE* input

int main (int argc, char* argv[])
{
  fprintf (stdout, "%% ");

  /* ¡REMPLIR-ICI! : Lire les commandes de l'utilisateur et les exécuter. */
  //int c;
  //int ret_code;
  int bufferSize = 255;
  char buffer[bufferSize];

  FILE* output = stdout;
  FILE* input = (argc > 1) ? fopen(argv[1], "r") : stdin;

  char* token;
  while(fgets(buffer, sizeof(buffer), input) && strcmp(buffer,"quit\n") != 0)
  {
     /* get the first token */
     token = strtok(buffer, " ");
     
     /* walk through other tokens */
     while( token != NULL ) 
     {
        fprintf(output, "%s\n", token);
        token = strtok(NULL, " ");
     }
    fprintf (stdout, "%% ");
  }

  fprintf (stdout, "Bye!\n");
  exit (0);
}