#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>
#include <sys/wait.h>

int main(void) {
    int catfd[2];
    if (pipe(catfd) < 0) {
        err(1, "failed pipe");
    }

    int pid = fork();
    if (pid < 0) {
        err(2, "failed fork");
    }

    if (pid == 0) {
        close(catfd[0]);
        if (dup2(catfd[1], 1) < 0) {
            err(3, "failed dup2");
        }
        close(catfd[1]);

        if (execlp("cat", "cat", "/etc/passwd", (char*)NULL) < 0) {
            err(4, "failed execlp of cat");
        }
    }

    close(catfd[1]);

    int sortfd[2];
    if (pipe(sortfd) < 0) {
        err(5, "failed pipe");
    }

    pid = fork();
    if (pid < 0) {
        err(6, "failed fork");
    }

    if (pid == 0) {
        close(sortfd[0]);
        if (dup2(catfd[0], 0) < 0) {
            err(7, "failed dup2");
        }
        if (dup2(sortfd[1], 1) < 0) {
            err(8, "failed dup2");
        }
        close(catfd[0]);
        close(sortfd[1]);

        if (execlp("sort", "sort", (char*)NULL) < 0) {
            err(9, "failed execlp of sort");
        }
    }

    close(sortfd[0]);
    close(catfd[1]);

    int uniqfd[2];
    if (pipe(uniqfd) < 0) {
        err(10 "failed pipe");
    }

    pid = fork();
    if (pid < 0) {
        err(11, "failed fork");
    }

    if (pid == 0) {
        close(uniqfd[0]);
        if (dup2(sortfd[0], 0) < 0) {
            err(12, "failed dup2");
        }
        close(sortfd[0]);
        if (dup2(uniqfd[1], 1) < 0) {
            err(13, "failed dup2");
        }
        close(uniqfd[1]);

        if (execlp("uniq", "uniq", "-c", (char*)NULL) < 0) {
            err(14, "failed execlp uniq");
        }
    }

    close(sortfd[0]);
    close(uniqfd[1]);

    while (wait(NULL) > 0);

    if (dup2(uniqfd[0], 0) < 0) {
        err(15, "failed dup2");
    }
    close(uniqfd[0]);

    if (execlp("sort", "sort", "-n", (char*)NULL) < 0) {
        err(16, "failed execlp of sort");
    }

}
