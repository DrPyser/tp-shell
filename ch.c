/* ch.c --- Un shell pour les hélvètes.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

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

int redirect(FILE* from, FILE* to){
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

int execProgram(char* pgmName, char* args[])
{
  int retCode, status;
  int pid = fork();
  if(pid == -1)
  {
      printf("PID -- I do not recognise the meaning of this word! -Margaret Tatcher");
      return -1;
  }
  if(pid == 0)
    retCode = execvp(pgmName,args); //on execute la commande dans le child
  else
    waitpid(pid, &status,WUNTRACED); //on attend que le child finisse
  return retCode;
}

int main (int argc, char* argv[])
{
  fprintf (stdout, "%% ");

  int bufferSize = 255;
  char buffer[bufferSize];
  char* tokenInput;

  FILE* output = stdout;
  FILE* input = (argc > 1) ? fopen(argv[1], "r") : stdin;

/*  if (path == NULL)
  {
    printf("There's no such thing as path. Only families and individuals. -Margaret Tatcher");
    return -1;
  }*/

  while(fgets(buffer, sizeof(buffer), input) && strcmp(buffer,"quit\n") != 0)
  {
     /* get the first tokenInput */
     tokenInput = strtok(buffer, " ");
     
     /* walk through other tokenInputs */
     while( tokenInput != NULL ) 
     {
        fprintf(output, "%s\n", tokenInput);
        tokenInput = strtok(NULL, " ");
     }
    fprintf (stdout, "%% ");
  }

  fprintf (stdout, "Bye!\n");
  exit (0);
}