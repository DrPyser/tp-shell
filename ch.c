/* ch.c --- Un shell pour les hélvètes.  */
/**
Écrit par Charles Langlois et François Poitras
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#define FALSE 0
#define TRUE 1
#define memcheck(var) do{\
        if(str == NULL){\
            fprintf(stderr, "There is no such thing as public memory; there is only \"%s\". -Margaret Thatcher", strerror(errno));\
            return NULL;\
        }\
    } while(0)

char last_directory_visited[1024];

int
fpeekc (FILE * f)
{
  int c = fgetc (f);
  if (!feof (f))
  {
    ungetc (c, f);
  }
  return c;
}

int
cd (char *path)
{
  int retCode;
  char *last = strdup (last_directory_visited);
  getcwd (last_directory_visited, 1024);
  if (path == NULL)
    retCode = chdir (getenv ("HOME"));
  else if (strcmp (path, "-") == 0)
    retCode = chdir (last);
  else
    retCode = chdir (path);
  if (retCode < 0)
    fprintf (stderr,
             "I don't think there will be a \"%s\" in my lifetime. -Margaret Tatcher\n",
             strerror (errno));
  return retCode;
}

/* Vérifie si le char 'c' est présent dans le string 'str'. Retourne 1 si c'est le cas, sinon 0*/
int
strmem (char *str, char c)
{
  int i = 0;
  int flag = 0;
  while (i < strlen (str) && !flag)
  {
    flag = flag || str[i++] == c;
  }
  return flag;
}

/* Reads stream 'src' into 'buffer' until either
 * the end-of-file or a character in 'seps' is encountered,
 * or until 'buffer' is full(according to 'size').
 */

int
readToken (FILE * src, char *buffer, int size)
{
  int i = 0;
  int c;
  char *delimiters = " |;<>\n";
  if (i < size)
  {
    while ((c = fgetc (src)) != EOF && strmem (delimiters, c));
    if (!feof (src))
    {
      do
      {
        buffer[i++] = c;
      }
      while ((c = fgetc (src)) != EOF && !strmem (delimiters, c)
             && i < size);
      if (i >= size)
      {
        fprintf (stderr,
                 "I like buffer overflows. We can do business together. -Margaret Tatcher\n");
        return -2;
      }
      else if (strmem (delimiters, c))
      {
        while (c == ' ' && !feof (src)
               && strmem (delimiters, fpeekc (src)))
          c = fgetc (src);
        buffer[i] = '\0';
        return c;
      }
    }
    else
      return EOF;
  }
  else
  {
    fprintf (stderr,
             "I like buffer overflows. We can do business together. -Margaret Tatcher\n");
    return -2;
  }
  return -1;
}

/* Reads a 'command'(a series of space-separated strings) into 'buffer'
 * until either a command-separating character(';','\n','|','>','<') or the end-of-input is encountered,
 * returning that character.
 */
int
readCommand (FILE * src, char *buffer, int size)
{
  int c;
  int i = 0;
  char *home;
  while ((c = fgetc (src)) == ' ');
  while (c != EOF && i < size)
  {
    //printf("\ncharacter read: %d\n", c);
    switch (c)
    {
    case '\\':
      c = fgetc (src);
      if (!feof (src))
      {
        buffer[i++] = c;
      }
      break;
    case '|':
    case '<':
    case '>':
    case ';':
    case '\n':
      buffer[i] = '\0';
      return c;
      break;
    case '~':
      //~ = $HOME
      home = getenv ("HOME");
      if (strlen (home) < size - i - 1)
      {
        int j = 0;
        while (i < size && home[j] != '\0')
          buffer[i++] = home[j++];
      }
      else
      {
        fprintf (stderr,
                 "I like buffer overflows. We can do business together. -Margaret Tatcher\n");
        return -2;
      }
      break;

    default:
      buffer[i++] = c;
      break;
    }
    c = fgetc (src);
  }
  if (i + 1 >= size)
  {
    fprintf (stderr,
             "I like buffer overflows. We can do business together. -Margaret Tatcher\n");
    return -2;
  }
  //encountered end-of-input
  else if (feof (src))
  {
    buffer[i] = '\0';
    return EOF;
  }
  return -1;
}

/*
 * Compte le nombre de jetons séparés par 'sep' dans le string 'str'
 */
int
countTokens (char *str, char sep)
{
  int i = 0;
  int count = 0;
  while (i < strlen (str))
  {
    if (str[i] != sep)
    {
      count++;
      while (++i < strlen (str) && str[i] != sep);
    }
    i++;
  }
  return count;
}

/* compte le nombre d'entités contenu par le dossier désigné par "path", excluant ceux dont le nom commence par '.' */
int
countDirectoryContent (char *path)
{
  //adapted from http://stackoverflow.com/questions/1121383/counting-the-number-of-files-in-a-directory-using-c
  int file_count = 0;
  DIR *dirp;
  struct dirent *entry;

  dirp = opendir (path);
  if (dirp)
  {
    while ((entry = readdir (dirp)) != NULL)
    {
      if (entry->d_name[0] != '.')
        file_count++;
    }
    closedir (dirp);
  }
  else
    fprintf (stderr,
             "No one would remember the Good Samaritan if he'd only had good intentions; he had the path as well.  -Margaret Tatcher\n");

  return file_count;
}

/*
  Adapté de http://stackoverflow.com/questions/8106765/using-strtok-in-c
  Sépare un string aux espaces, et retourne un tableau contenant les strings résultants
*/
char **
tokenize (const char *input)
{
  char *str = strdup (input);
  memcheck (str);
  int count = 0;
  int capacity = countTokens (str, ' ') + 1;
  char **result = malloc (capacity * sizeof (char *));
  memcheck (result);

  char *tok = strtok (str, " ");

  while (tok != NULL)
  {
    //Si l'argument '*' est utilisé, on ajoute aux arguments le contenu du dossier actuel
    if (strcmp (tok, "*") == 0)
    {
      char cwd[258];
      if (getcwd (cwd, 258) != NULL)
      {
        capacity += countDirectoryContent (cwd);
        result = realloc (result, capacity * sizeof (char *));
        memcheck (result);
        DIR *d;
        struct dirent *dir;
        d = opendir (".");
        if (d)
        {
          while ((dir = readdir (d)) != NULL)
          {
            if (dir->d_name[0] != '.')
            {
              result[count++] = strdup (dir->d_name);
            }
          }
          closedir (d);
        }
        else
        {
          fprintf (stderr,
                   "Where there is discord, may we bring harmony. Where there is \"%s\", may we bring truth.\n\
 Where there is doubt, may we bring faith. And where there is despair, may we bring hope. -Margaret Tatcher\n",
                   strerror (errno));
          return NULL;
        }
      }
      else
      {
        fprintf (stderr,
                 "I like \"%s\". We can do business together. -Margaret Tatcher\n",
                 strerror (errno));
        return NULL;
      }
    }
    else
    {
      result[count++] = (char *) (tok ? strdup (tok) : tok);
    }
    tok = strtok (NULL, " ");
  }
  result[count] = (char *) NULL;
  free (str);
  return result;
}

/* Prend une liste de string en argument, et exécute la commande correspondant au premier string dans un child process */
int
execCommand (char **args, int in, int out)
{
  int retCode, status, i;
  if (args[0] == NULL)
    return -1;
  else if (strcmp (args[0], "cd") == 0)
  {
    retCode = cd (args[1]);
    return retCode;
  }
  else
  {
    int pid = fork ();

    if (pid < 0)
    {
      fprintf (stderr,
               "PID -- I do not recognise the meaning of this word! -Margaret Tatcher\n");
      return -1;
    }
    if (pid == 0)
    {
      if (in != 0)
      {
        dup2 (in, 0);
        close (in);
      }
      if (out != 1)
      {
        dup2 (out, 1);
        close (out);
      }
      retCode = execvp (args[0], args); //On execute la commande dans le child

      if (retCode == -1)
      {
        i = 0;
        while (args[i] != NULL)
        {
          free (args[i]);
          i++;
        }
        exit (errno); //As a result of its failure, the child commits Seppuku
      }
    }
    else
    {
      waitpid (pid, &status, WUNTRACED);  //On attend que le child finisse
      if (WIFEXITED (status))
        return WEXITSTATUS (status);
      else
        return -1;
    }
    return retCode;
  }
}

void
interrupt_signal_handler (int signum)
{
  if (signum == SIGINT)
  {
    printf
    ("\nMy job is to stop the command from completing. -Margaret Thatcher\n");
  }

}

int
main (int argc, char *argv[])
{
  int bufferSize = 1024;
  char cwd[bufferSize];
  char buffer[bufferSize];  //Buffer used to read a command
  char filename[bufferSize];  //buffer used to read a filename for a redirection
  int quit = FALSE;   //quit flag. If = 1, the program will exit the read-execute loop and terminate
  int execute = FALSE;    //skip flag. If true(1), the command will be executed directly.
  int abort = FALSE;    //skip flag. If true(1), the command will not be executed.
  int flag;     //flag returned by "readCommand" indicating the delimiter character terminating the command, or an error code
  int fd[2];      //pipes
  int in = 0;     //input file descriptor
  int out = 1;      //output file descriptor
  char **args;      //tokenized command
  char *mode;     //file open mode for redirection("a" or "w")
  FILE *fileptr;
  int i;      //Used for iterating through arguments
  signal (SIGINT, interrupt_signal_handler);  //handler pour control-c

  getcwd (cwd, sizeof (cwd));
  strcpy (last_directory_visited, cwd);
  fprintf (stdout, "The fightback begins now! --Margaret Thatcher\n");
  fprintf (stdout, "%s %% ", cwd);
  while (!quit)
  {
    execute = FALSE;
    abort = FALSE;
    //reads a command, up until end of line or command separating character('|',';')
    pipe (fd);    //Setting up pipelines for processes communication
    flag = readCommand (stdin, buffer, bufferSize); //Lit une commande jusqu'à rencontré ';','<','>', '\n' ou '|'
    out = 1;
    quit = feof (stdin) || strcmp (buffer, "quit") == 0;
    if (quit)
      fprintf (stdout,
               "\nIt is a great night. It is the end of this shell. --Margaret Thatcher\n");
    else
    {
      //fini la lecture de la commande(incluant les redirections) jusqu'à rencontré ';','|' ou '\n'
      while (!(execute || abort))
      {
        switch (flag)
        {
        case EOF:
          quit = TRUE;
          abort = TRUE;
          break;
        case -2:
          //Buffer overflow
          abort = TRUE;
          break;
        case '|':
          //Piping
          out = fd[1];
          execute = TRUE;
          break;
        case '>':
          //output redirection
          if (fpeekc (stdin) == '>')
          {
            fgetc (stdin);
            mode = "a";
          }
          else
            mode = "w";

          flag = readToken (stdin, filename, bufferSize);
          fileptr = fopen (filename, mode);
          if (fileptr != NULL)
            out = fileno (fileptr);
          else
          {
            fprintf (stderr,
                     "Where there is discord, may we bring harmony. Where there is \"%s\", may we bring truth.\n\
 Where there is doubt, may we bring \"%s\".\
 And where there is despair, may we bring hope. -Margaret Tatcher\n",
                     strerror (errno), filename);
            abort = TRUE;
          }
          break;
        case '<':
          //input redirection
          flag = readToken (stdin, filename, bufferSize);
          if (access (filename, R_OK) != -1)
          { //Si le fichier est accessible
            in = fileno (fopen (filename, "r"));
          }
          else
          {
            fprintf (stderr,
                     "Where there is discord, may we bring harmony.\
 Where there is \"%s\", may we bring truth.\n Where there is doubt, may we bring \"%s\".\
 And where there is despair, may we bring hope. -Margaret Tatcher\n", strerror (errno), filename);
            abort = TRUE;
          }
          break;
        case ';':
        case '\n':
          //out = 1;
          execute = TRUE;
          break;
        case 3:
          abort = TRUE;
          break;
        }
      }
      if (!(quit || abort))
      {
        args = tokenize (buffer);
        if (args == NULL)
          fprintf (stderr, "Tokenizer failure");
        else if (args[0] == NULL)
        { //la commande est vide
          if (flag != '\n')
          {
            fprintf (stderr,
                     "In my lifetime all our problems have come from unexpected \"%c\"\n\
 and all the solutions have come from the english-speaking countries across the world. -Margaret Tatcher\n",
                     flag);
            while (!feof (stdin) && fgetc (stdin) != '\n'); //On vide le input stream
          }
        }
        else
        {
          if (execCommand (args, in, out) == ENOENT)
          {
            fprintf (stderr,
                     "There's no such thing as \"%s\". Only families and individuals. -Margaret Tatcher.\n",
                     args[0]);
          }
          i = 0;
          while (args[i] != NULL)
          {
            free (args[i]);
            i++;
          }
        }
      }
      if (!quit)
      {
        //Post-exécution de la commande
        switch (flag)
        {
        case '|':
          close (fd[1]);
          in = fd[0]; //rotation de l'input pour la prochaine commande
          break;
        case 3:
        case '\n':
          in = 0; //retour au stdin par défaut
          getcwd (cwd, sizeof (cwd)); //Au cas ou le répertoire courrant à été changé par la commande
          fflush (stdout);
          fprintf (stdout, "%s %% ", cwd);
          break;
        default:
          in = 0;
          break;
        }
      }
    }

  }
  close (fd[0]);
  close (fd[1]);
  exit (EXIT_SUCCESS);
}
