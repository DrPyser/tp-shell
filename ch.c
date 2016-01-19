/* ch.c --- Un shell pour les hélvètes.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Pour les tests.
#define memcheck(x) do{\
  if(x == NULL) {				\
    fprintf(stderr, "Mémoire epuisée.\n");\
    exit(1);\
  }\
  }while(0)

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
    fprintf(stderr, "PID -- I do not recognise the meaning of this word! -Margaret Tatcher");
      return -1;
  }
  if(pid == 0)
    retCode = execvp(pgmName,args); //on execute la commande dans le child
  else
    waitpid(pid, &status,WUNTRACED); //on attend que le child finisse
  return retCode;
}

int countTokens(char* str, char sep){    
  int i = 0;

  while(i < strlen(str) && str[i] == sep)
    i++;

  if(i == strlen(str))
    return 0;

  int count = 1;
  while(i < strlen(str)){
    if(str[i++] == sep){
      count++;
      while(str[i] < strlen(str) && str[i++] == sep);
    }
  }
  
  return count;
}

/*
char* strdup(char* str){
  char* dup = malloc(strlen(str));
  return strcpy(dup,str);
}
*/

int main (int argc, char* argv[])
{
  //printf("%d\n", countTokens("1", ' '));
  fprintf (stdout, "%% ");

  int bufferSize = 255;
  char buffer[bufferSize];
  char* tokenInput;
  //int c;
  char* name;
  char** args;

  FILE* output = stdout;
  FILE* input = (argc > 1) ? fopen(argv[1], "r") : stdin;

  /*  if (path == NULL)
  {
    printf("There's no such thing as path. Only families and individuals. -Margaret Tatcher");
    return -1;
    }*/

  while(fgets(buffer, bufferSize, input) != NULL && strcmp(buffer,"quit\n") != 0)
    {
      
      int i = 0;

      args = malloc((countTokens(buffer,' ') + 1) * sizeof(char*));
      memcheck(args);

      //get the first token
      tokenInput = strtok(buffer, " \n");
      if(tokenInput != NULL){
	name = strdup(tokenInput);
	memcheck(name);
	
	//walk through other tokens
	i = 0;
	while( tokenInput != NULL ) 
	  {	  
	    //fprintf(stdout, "%s\n", tokenInput);
	    args[i++] = strdup(tokenInput);
	    tokenInput = strtok(NULL, " \n");
	  }
	args[i] = (char*) NULL;
	execProgram(name,args);

	free(name);
	i = 0;
	while(args[i] != NULL)
	  free(args[i++]);
	
      }
      fprintf(stdout,"%% ");          
      
    }
  
  fprintf (stdout, "Bye!\n");
  exit (0);
}
