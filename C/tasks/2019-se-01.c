#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        errx(1, "invalid argument count");
    }

    char* args[argc];
    for (int i = 2; i < argc; i++) {
        args[i-2] = argv[i];
    }
    args[argc - 2] = NULL;

    int sec = atoi(argv[1]);

    int fd = open("run.log", O_WRONLU | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        err(2, "failed open");
    }

    int c = 0;

    while (1) {

        time_t start_time = time(NULL);
        int pid = fork();

        if (pid < 0) {
            err(3, "failed fork");

        }
        if (pid == 0) {
            execvp(argv[2], args);
            err(4, "failed exec");
        }

        int status;
        if (wait(&status) < 0) {
            close(fd);
            err(3, "couldnt wait");
        }

        time_t end_time = time(NULL);
        int exit_code = 0;

        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            exit_code = 129;
        }

        if (dprintf(fd, "%ld %ld %d\n", (long)start_time, (long)end_time, exit_code)) {
            err(4, "failed logging");
        }

        if (exit_code != 0 && end_time - start_time < sec) {
            c++;
        } else {
            c = 0;
        }

        if (c == 2) {
            break;
        }

    }

    close(fd);
    return 0;
}
