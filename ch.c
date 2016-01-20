/* ch.c --- Un shell pour les hélvètes.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//pour les process
#include <sys/wait.h>
#include <unistd.h>

//pour les directory
#include <dirent.h>

// Pour les tests.
#define memcheck(x) do{\
  if(x == NULL) {       \
    fprintf(stderr, "Mémoire epuisée.\n");\
    exit(1);\
  }\
  }while(0)

/**
 *Writes the data from the stream "from" to a file named "filename".
 *If "filename" designates an already existing file, the file is overwritten.
 *Otherwise, a new file is created.
*/
int outputToFile(FILE* from, char* filename) {
  FILE* output = fopen(filename, "w");
  int c;
  int ret_code;
  while ((c = fgetc(from)) != EOF) {
    ret_code = fputc(c, output);
    if (ret_code == EOF) {
      fprintf(stderr, "failure to write character %c in file %s", c, filename);
      fclose(output);
      return EXIT_FAILURE;
    }
  }

  fflush(output);
  fclose(output);

  return EXIT_SUCCESS;
}

int appendToFile(FILE* from, char* filename) {
  FILE* output = fopen(filename, "a");
  int c;
  int ret_code;
  while ((c = fgetc(from)) != EOF) {
    ret_code = fputc(c, output);
    if (ret_code == EOF) {
      fprintf(stderr, "failure to write character %c in file %s", c, filename);
      fclose(output);
      return EXIT_FAILURE;
    }
  }

  fflush(output);
  fclose(output);

  return EXIT_SUCCESS;
}

int countFiles (char *path)
{
  return 0;
}

void argExpension(char *args[])
{
  char *pwd = getenv("PWD");
  int i = 0;
  if(pwd == NULL)
  {
    fprintf(stderr, "Oh, but you know, you do not achieve anything without PWD, ever. -Margaret Tatcher");
    return;
  }
  DIR *d;
  struct dirent *dir;
  d = opendir(pwd);
  if(d)
  {
   while((dir = readdir(d)) != NULL)
   {
        args[i++] = strdup(dir->d_name);
   }
   closedir(d);
  }
}

int redirect(FILE* from, FILE* to) {
  int c;
  int ret_code;
  while ((c = fgetc(from)) != EOF) {
    ret_code = fputc(c, to);
    if (ret_code == EOF) {
      fprintf(stderr, "failure to write character %c to output stream", c);
      return EXIT_FAILURE;
    }
  }
  fflush(from);
  return EXIT_SUCCESS;
}

int execProgram(char* pgmName, char* args[], int argCount)
{
  int retCode, status;
  int pid = fork();

  
  // int redirectPosition = -1;
  // char *last = args[argCount];
  // char *beforeLast;
  // char firstChar = last[0];
  // char lastChar;

  // if(argCount > 1)
  // {
  //   beforeLast = args[argCount-1];
  //   lastChar = beforeLast[strlen(beforeLast)-1];

  // }
  // else
  // {

  // }
  if (pid == -1)
   {
     fprintf(stderr, "PID -- I do not recognise the meaning of this word! -Margaret Tatcher");
     return -1;
   }
   if (pid == 0)
   { //on execute la commande dans le child
  //   switch(redirectPosition)
  //   {
  //     case 1:
  //     case 2:
  //     case 3:
  //     default:
         retCode = execvp(pgmName, args);
    }
   
  // } 
  else
    waitpid(pid, &status, WUNTRACED); //on attend que le child finisse
  return retCode;
}

int countTokens(char* str, char sep) {
  int i = 0;

  while (i < strlen(str) && str[i] == sep)
    i++;

  if (i == strlen(str))
    return 0;

  int count = 1;
  while (i < strlen(str)) {
    if (str[i++] == sep) {
      count++;
      while (str[i] < strlen(str) && str[i++] == sep);
    }
  }

  return count;
}

int main (int argc, char* argv[])
{
  //printf("%d\n", countTokens("1", ' '));
  fprintf (stdout, "%% ");

  int bufferSize = 255;
  char buffer[bufferSize];
  char *tokenInput, *name;
  char **args;

  FILE* output = stdout;
  FILE* input = (argc > 1) ? fopen(argv[1], "r") : stdin;

  while (fgets(buffer, bufferSize, input) != NULL && strcmp(buffer, "quit\n") != 0)
  {

    int i = 0;
    int nbTokens = countTokens(buffer, ' ') + 1; //we will need this later
    args = malloc(nbTokens * sizeof(char*));
    memcheck(args);

    //get the first token
    tokenInput = strtok(buffer, " \n");
    if (tokenInput != NULL) {
      name = strdup(tokenInput);
      memcheck(name);

      //walk through other tokens
      i = 0;
      while ( tokenInput != NULL )
      {
        //fprintf(stdout, "%s\n", tokenInput);
        args[i++] = strdup(tokenInput);
        tokenInput = strtok(NULL, " \n");
      }
      args[i] = (char*) NULL;

      if(strcmp(args[i-1], "*") == 0)
      {
        argExpension(args);
      }
      execProgram(name, args, i-1);

      free(name);
      i = 0;
      while (args[i] != NULL)
        free(args[i++]);

    }
    fprintf(stdout, "%% ");
    free(args);
  }

  fprintf (stdout, "Bye!\n");
  exit (0);
}
