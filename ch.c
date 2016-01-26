/* ch.c --- Un shell pour les hélvètes.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

// Pour les tests.
#define memcheck(x) do{                             \
        if(x == NULL) {                             \
            fprintf(stderr, "Mémoire epuisée.\n");	\
            exit(1);                                \
        }                                           \
    }while(0)

#define FALSE 0
#define TRUE 1

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

/* Vérifi si le char 'c' est présent dans le string 'str'. Retourne 1 si c'est le cas, sinon 0*/
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
        case '\\':
            c = fgetc(src);
            if(!feof(src)){
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
    else if(feof(src)){
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

/* compte le nombre d'entités contenu par le dossier désigné par "path" */
int countDirectoryContent(char *path)
{

    //taken from http://stackoverflow.com/questions/1121383/counting-the-number-of-files-in-a-directory-using-c
    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;

    dirp = opendir(path);
    if (dirp)
    {
        while ((entry = readdir(dirp)) != NULL)
        {
            if (entry->d_type == DT_REG)
                file_count++;
        }
        closedir(dirp);
    }
    else
        fprintf(stderr, "There's no such thing as a path. Only families and individuals. -Margaret Tatcher");

    return file_count;
}

/*
  http://stackoverflow.com/questions/8106765/using-strtok-in-c
  Sépare le string reçu en entré aux espaces, et retourne un pointeur vers un array contenant les strings résultant
*/
char** tokenize(const char* input)
{
    char* str = strdup(input);
    int count = 0;
    int capacity = countTokens(str, ' ') + 1;
    char** result = malloc(capacity * sizeof(char*));

    char* tok = strtok(str, " ");

    while (tok != NULL) {
        //Si l'argument '*' est utilisé, on ajoute aux arguments le contenu du dossier actuel
        if (strcmp(tok, "*") == 0) {
            capacity += countDirectoryContent(getenv("PWD"));
            result = realloc(result, capacity * sizeof(char*));
            DIR           *d;
            struct dirent *dir;
            d = opendir(".");
            if (d) {
                while ((dir = readdir(d)) != NULL)
                {
                    if (dir->d_name[0] != '.')
                        result[count++] = strdup(dir->d_name);
                }
                closedir(d);
            }
        } else{
            //if(tok[strlen(tok)-1] == '\\'){
            result[count++] = (char*) (tok ? strdup(tok) : tok);
        }
        tok = strtok(NULL, " ");
    }
    result[count] = (char*) tok;
    free(str);
    return result;
}

/* Prend une liste de string en argument, et exécute la commande correspondant au premier string dans un child process */
int execCommand(char** args, int in, int out)
{
    int retCode, status;
    if(strcmp(args[0],"cd") == 0){
        retCode = cd(args[1]);
        return retCode;
    } else {    
        int pid = fork();
	
        if(pid < 0)
        {
            fprintf(stderr, "PID -- I do not recognise the meaning of this word! -Margaret Tatcher");
            return -1;
        }
        if(pid == 0){
            if(in != 0){
                dup2(in,0);
                close(in);
            }
            if(out != 1){
                dup2(out,1);
                close(out);
            }
            retCode = execvp(args[0],args); //on execute la commande dans le child
	    
            if(retCode == -1)
                raise(SIGTERM);//As a result of its failure, the child commits Seppuku
        }
        else
            //if(out == 1)//Si le output n'est pas stdout, le prochain processus sera directement exécuté
            waitpid(pid, &status, WUNTRACED); //on attend que le child finisse
        return retCode;
    }
    
}

int main (int argc, char* argv[])
{
    char cwd[1024];
    getcwd(cwd,sizeof(cwd));
    fprintf (stdout, "%s\n%% ", cwd);
    int bufferSize = 255;
    char buffer[bufferSize];
    char filename[bufferSize];
    int quit = 0;//quit flag;
    int flag;
    int fd[2];//File descriptors for input/output
    int in = 0;//input file descriptor
    int out = 1;
    char** args;
    
    while(!quit){
        pipe(fd);//Setting up pipelines for processes communication
        flag = readCommand(stdin, buffer, bufferSize);//lit une commande
        quit = feof(stdin) || strcmp(buffer,"quit") == 0;//
        if(!quit){
            printf("Buffer: %s\n", buffer);	
            args = tokenize(buffer);
            char* mode;
            if(args[0] != NULL){
                int i = 0;
                while(args[i] != NULL)
                    printf("%s\n",args[i++]);
		
                switch(flag){
                case EOF:
                    quit = 1;
                    break;
                case -2:
                    //Do something?
                    break;    
                case '|':
                    out = fd[1];		    
                    break;
                case '>':
                    //do redirection		    
                    if(fpeekc(stdin) == '>'){
                        fgetc(stdin);
                        mode = "a";				
                    }
                    else
                        mode = "w";
                    flag = readToken(stdin,filename," ;\n<>|",bufferSize);
                    out = fileno(fopen(filename,mode));
                    //Exécute la commande avec le file descriptor du fichier comme output
                    break;
                case '<':
                    //same
                    flag = readToken(stdin,filename," ;\n<>|",bufferSize);
                    in = fileno(fopen(filename,"r"));
                    break;
                case ';':
                case '\n':		    
                    out = 1;
                    break;
                }
                if(!quit)
                    execCommand(args, in, out);
                i = 0;
                while(args[i] != NULL){
                    //fprintf(stdout,"Freeing %s\n", args[i]);
                    free(args[i]);
                    i++;
                }
            }
            if(!quit){
                switch(flag){
                case '|':
                    if(args[0] == NULL){
                        fprintf(stderr, "syntax error: unexpected token '|'.");
                    }
                    else
                        close(fd[1]);
                    in = fd[0];
                    break;
                case '\n':
                    out = 1;
                    in = 0;
                    getcwd(cwd,sizeof(cwd));
                    fflush(stdout);
                    fprintf(stdout,"%% ");
                    break;
                default:
                    out = 1;
                    in = 0;
                    break;
                }
            }
	    
        }
    }
    fprintf (stdout, "\nBye!\n");
    exit(0);
}
