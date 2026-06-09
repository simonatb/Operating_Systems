## Numbers
atoi() - convert a string to an integer

strtol(char* num, endptr, int base)  - convert a string to a long integer
## Pipe
mkfifo(const char *pathname, mode_t mode) 

unlink(const char* pathname)

dup2(int oldfd, int newfd) - newfd becomes an alias for oldfd 

after that: close(oldfd)
## Exec
### execl - пълен път, аргументи като списък
``` execl("/bin/ls", "ls", "-l", "-a", (char*)NULL); ```
    
### execlp - търси в PATH
``` execlp("ls", "ls", "-l", (char*)NULL);``` 
    
### execv - аргументи като масив
```
char *args[] = {"ls", "-l", "-a", (char*)NULL};

execv("/bin/ls", args);
```     
### execvp - масив + търсене в PATH
``` execvp("ls", args);``` 
    
### execle - със собствен environment
```
char *env[] = {"HOME=/tmp", "USER=test", (char*)NULL};
execle("/bin/ls", "ls", "-l", (char*)NULL, env);
```
