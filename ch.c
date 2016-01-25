/* ch.c --- Un shell pour les hélvètes.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

// Pour les tests.
#define memcheck(x) do{					\
	if(x == NULL) {					\
	    fprintf(stderr, "Mémoire epuisée.\n");	\
	    exit(1);					\
	}						\
    }while(0)

/**
 *Writes the data from the stream "from" to a file named "filename".
 *If "filename" designates an already existing file, the file is overwritten.
 *Otherwise, a new file is created.
 */

int fpeekc(FILE* f){
    int c = fgetc(f);
    if(!feof(f)){
	ungetc(c, f);
    }
    return c;
}

int cd(char* path){
    int retCode;
    if(path == NULL)
	retCode = chdir(getenv("HOME"));
    else
	retCode = chdir(path);

    if(retCode < 0)
      fprintf(stderr,"I don't think there will be a \"%s\" in my lifetime. -Margaret Tatcher\n",strerror(errno));
    return retCode;
}

int strmem(char* str, char c){
    int i = 0;
    int flag = 0;
    while(i < strlen(str) && !flag){
	flag = flag || str[i++] == c;
    }
    return flag;
}

/* Reads stream 'src' into 'buffer' until either 
 * the end-of-file or a character in 'seps' is encountered, 
 * or until 'buffer' is full(according to 'size').
 */
int readToken(FILE* src, char* buffer, char* seps, int size){
    int i = 0;
    int c;    
    if(i < size){
	while((c = fgetc(src)) != EOF && strmem(seps, c));
	if(!feof(src)){
	    do{
		buffer[i++] = c;
	    } while((c = fgetc(src)) != EOF && !strmem(seps, c) && i < size);
	    if(i >= size){
		fprintf(stderr, "Buffer overflow!");
		return -2;
	    }
	    else if(strmem(seps, c)){
		buffer[i] = '\0';
		return c;
	    }
		
	} else
	    return EOF;	
    }
    else{
	fprintf(stderr, "Buffer overflow!");
	return -2;
    }
    return -1;
}

/* Reads a 'command'(a series of space-separated strings) into 'buffer' 
 * until either a command-separating character(';','\n','|','>','<') or the end-of-input is encountered, 
 * returning that character.
 */
int readCommand(FILE* src, char* buffer, int size){
    int c;
    int i = 0;
    char* home;
    while((c = fgetc(src)) == ' ');
    while(c != EOF && i < size){
	switch(c){
	case '|':
	case '<':
	case '>':
	case ';':
	case '\n':
	    buffer[i] = '\0';
	    return c;
	    break;
	case '~':
	    home = getenv("HOME");
	    if(strlen(home) < size-i-1){
		int j = 0;
		while(i < size && home[j] != '\0')
		    buffer[i++] = home[j++];
	    }
	    else{
		fprintf(stderr, "Buffer overflow.");
		return -2;
	    }
	    break;
		
	default:
	    buffer[i++] = c;
	    break;
	}
	c = fgetc(src);
    }

    //The buffer overflowed
    if(i >= size && feof(src) != 0){
	fprintf(stderr, "Buffer overflow; Command too big.");
	return -2;
    }
    //encountered end-of-input
    else if(c == EOF){
	buffer[i] = '\0';
	return EOF;
    }
    return -1;
}


int countTokens(char* str, char sep){    
    int i = 0;
    int count = 0;
    while(i < strlen(str)){
	if(str[i] != sep){
	    count++;
	    while(++i < strlen(str) && str[i] != sep);
	}
	i++;
    }
    return count;
}


/*Counts the content of the directory given by 'path'.
 *Ignores entities whose name begins with '.'
 */
int countDirectoryContent(char *path)
{

    //taken from http://stackoverflow.com/questions/1121383/counting-the-number-of-files-in-a-directory-using-c
    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;
    
    dirp = opendir(path); /* There should be error handling after this */
    while ((entry = readdir(dirp)) != NULL) 
	{
      if(entry->d_name[0] != '.')
	  file_count++;      
	}
    closedir(dirp);
    return file_count;
}


/* Like 'countDirectoryContent' but includes entities beginning with '.'
 */
int countAllDirectoryContent(char *path)
{

  //taken from http://stackoverflow.com/questions/1121383/counting-the-number-of-files-in-a-directory-using-c
  int file_count = 0;
  DIR * dirp;
  struct dirent * entry;

  dirp = opendir(path); /* There should be error handling after this */
  while ((entry = readdir(dirp)) != NULL) 
  {
      file_count++;
  }
  closedir(dirp);
  return file_count;
}

/*
http://stackoverflow.com/questions/8106765/using-strtok-in-c
*/
char** tokenize(const char* input)
{
    char* str = strdup(input);
    int count = 0;
    int capacity = countTokens(str,' ') + 1;
    char** result = malloc(capacity*sizeof(char*));

    char* tok = strtok(str," "); 

    while(tok != NULL){
	//Si l'argument '*' est utilisé, on ajoute le contenu du dossier actuel
	if(strcmp(tok, "*") == 0){
	    capacity += countDirectoryContent(getenv("PWD"));
	    result = realloc(result, capacity*sizeof(char*));
	    DIR           *d;
	    struct dirent *dir;
	    d = opendir(".");
	    if (d){
		while((dir = readdir(d)) != NULL)
		    {
			if(dir->d_name[0] != '.')
			    result[count++] = strdup(dir->d_name);			
		    }
		closedir(d);
	    }
	} else
	    result[count++] = (char*) (tok? strdup(tok) : tok);
	tok = strtok(NULL," ");
    }
    result[count] = (char*) tok;
    //free(str);
    return result;
}

int execCommand(char* command)
{
    int retCode, status;
    char** args = tokenize(command);	
    int i;
    
    if(args[0] != NULL){
	if(strcmp(args[0],"cd") == 0){
	    retCode = cd(args[1]);
	    return retCode;
	}
	else{    
	    int pid = fork();
	
	    if(pid == -1)
		{
		    fprintf(stderr, "PID -- I do not recognise the meaning of this word! -Margaret Tatcher");
		    return -1;
		}
	    if(pid == 0){
		//Parse the command and expand the '*' argument 	    
	    
		retCode = execvp(args[0],args); //on execute la commande dans le child
		i = 0;
		while(args[i] != NULL){
		    //fprintf(stdout,"Freeing %s\n", args[i]);
		    free(args[i]);
		    i++;
		}
		if(retCode == -1)
		    raise(SIGTERM);//As a result of its failure, the child commits Seppuku
	    }
	    else
		waitpid(pid, &status,WUNTRACED); //on attend que le child finisse
	    return retCode;
	}
    }
    else
	return -1;
}

int main (int argc, char* argv[])
{
    //printf("%d\n", countTokens(" 1 2 3", ' '));
    fprintf (stdout, "%% ");

    int bufferSize = 255;
    char buffer[bufferSize];
    int quit = 0;//quit flag;
    //int fd[2];//File descriptors for input/output
    
    while(!quit){
	switch(readCommand(stdin, buffer, bufferSize)){
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
	    //char filename[bufferSize];
	    //int c = fpeekc(stdin);
	    //char* badchars = "<";//charactères interdits dans le nom d'un fichier
	    //if(c == '>'){
	    //	readToken(stdin, filename, " ;|><\n", bufferSize);
		//open file 'filename' in 'append' mode
	    //}
	    break;
	case '<':
	    //same
	case ';':
	    quit = strcmp(buffer,"quit") == 0;    
	    if(!quit){
		printf("Buffer: %s\n", buffer);
		execCommand(buffer);		
	    }
	    break;
	case '\n':
	    quit = strcmp(buffer,"quit") == 0;    
	    if(!quit){
		printf("Buffer: %s\n", buffer);
		execCommand(buffer);//Parse and execute the command in a new child process		
		fprintf(stdout,"%% ");          		
	    }
	    break;    
	}
    }
    fprintf (stdout, "Bye!\n");
    exit(0);
}
