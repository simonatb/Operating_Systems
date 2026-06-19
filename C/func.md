## Numbers
atoi() - convert a string to an integer

strtol(char* num, endptr, int base)  - convert a string to a long integer

qsort()
```
int comp(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int main() {
    int arr[] = {5, 2, 3, 1, 4};
    int n = sizeof(arr) / sizeof(arr[0]);

    qsort(arr, n, sizeof(arr[0]), comp);

    return 0;
}
```
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
## Temp files
mkstemp(/tmp/name_XXXXXX) - generates a unique temporary filename from template
```
char template[] = "/tmp/tempfile_XXXXXX";
int fd = mkstemp(template);

if (fd == -1) {
    perror("Грешка при създаване на временен файл");
    return 1;
}

close(fd);
unlink(template);
```
## Byte conversions
1KB - 1 000 bytes

1MB - 1 000 000 bytes

1GB - 1 000 000 000 bytes
## Watchdog
```
int status;
int dead = waitpid(-1, &status, WNOHANG);
if (dead > 0) {
    for (int i = 0; i < 3; i++) {
        if (pids[i] == dead) {
            fprintf(stderr, "sensor %d died\n", i);
            start_sensor(i);
            break;
        }
    }
}
```
## Format printing
int snprintf(char *buffer, size_t size, const char *format, ...); size- with the terminating zero; to a string

int fprintf(FILE *stream, const char *format, ...); - to a stream

