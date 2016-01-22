/* ch.c --- Un shell pour les hélvètes.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Pour les tests.
#define memcheck(x) do{         \
  if(x == NULL) {         \
      fprintf(stderr, "Mémoire epuisée.\n");  \
      exit(1);          \
  }           \
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

int execProgram(char* pgmName, char* args[])
{
  int retCode, status;
  int pid = fork();
  if (pid == -1)
  {
    fprintf(stderr, "PID -- I do not recognise the meaning of this word! -Margaret Tatcher");
    return -1;
  }
  if (pid == 0) {
    retCode = execvp(pgmName, args); //on execute la commande dans le child
    if (retCode == -1) {
      raise(SIGTERM);//As a result of its failure, the child commits Seppuku
    }
  }
  else
    waitpid(pid, &status, WUNTRACED); //on attend que le child finisse
  return retCode;
}

/**
 * Read tokens into 'buffer' until a command-separating character,
 * such as '|', '<', '>', ';' or '\n' and returns it.
 */
int readCommand(FILE* src, char* buffer, int size) {
  int c;
  int i = 0;
  while ((c = fgetc(src)) == ' ');
  while (c != EOF && i < size) {
    switch (c) {
    case '|':
    case '<':
    case '>':
    case ';':
    case '\n':
      buffer[i] = '\0';
      return c;
      break;
    default:
      buffer[i++] = c;
      break;
    }
    c = fgetc(src);
  }

  if (i >= size && feof(src) != 0) {
    fprintf(stderr, "Buffer overflow; Command too big.");
    return -2;
  }
 // else if (c == EOF) {
    buffer[i] = '\0';
    return EOF;
//  }
}

int countTokens(char* str, char sep) {
  int i = 0;
  int count = 0;
  while (i < strlen(str)) {
    if (str[i] != sep) {
      count++;
      while (++i < strlen(str) && str[i] != sep);
    }
    i++;
  }
  return count;
}


/*
http://stackoverflow.com/questions/8106765/using-strtok-in-c
*/
char** tokenize(const char* input)
{
  char* str = strdup(input);
  int count = 0;
  int capacity = countTokens(str, ' ') + 1;
  char** result = malloc(capacity * sizeof(char*));

  char* tok = strtok(str, " ");

  while (tok != NULL) {
    //if (count >= capacity)
    //  result = realloc(result, (capacity*=2)*sizeof(*result));
    result[count++] = (char*) (tok ? strdup(tok) : tok);
    tok = strtok(NULL, " ");
  }
  result[count] = (char*) tok;
  free(str);
  return result;
}

/*Étant donné un string correspondant à une commande,
 *divise la commande en 'tokens' aux espaces, et exécute la commande dans un child process.
 */
int parseAndRun(char* buffer) {
  char** args;
  int i = 0;
  int retcode = 0;

  args = tokenize(buffer);
  if (args[0] != NULL) {
    retcode = execProgram(args[0], args);
    while (args[i] != NULL) {
      //fprintf(stdout,"Freeing %s\n", args[i]);
      free(args[i]);
      i++;
    }
  }
  free(args);

  return retcode;
}

int main (int argc, char* argv[])
{
  //printf("%d\n", countTokens(" 1 2 3", ' '));
  fprintf (stdout, "%% ");

  int bufferSize = 255;
  char buffer[bufferSize];
  int quit = 0;//quit flag;
  //int fd[2];//File descriptors for input/output

  while (!quit) {
    switch (readCommand(stdin, buffer, bufferSize)) {
    case EOF:
      quit = 1;
      break;
    case -2:
      //Do something?
      break;
    case '|':
    /*
    pipe(fd);//Setting up pipelines for process
    dup2(0,fd[0]);
    dup2(1,fd[1]);
    */
    case '>':
    //do redirection
    case '<':
    //same
    case ';':
      quit = strcmp(buffer, "quit") == 0;
      if (!quit) {
        printf("Buffer: %s\n", buffer);
        parseAndRun(buffer);
      }
      break;
    case '\n':
      quit = strcmp(buffer, "quit") == 0;
      if (!quit) {
        printf("Buffer: %s\n", buffer);
        parseAndRun(buffer);
        fprintf(stdout, "%% ");
      }
      break;
    }
  }
  fprintf (stdout, "Bye!\n");
  exit(0);
}
