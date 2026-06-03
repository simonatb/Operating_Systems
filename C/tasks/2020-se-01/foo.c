#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <err.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

void asserted_wait(void) {
    int status;
    if (wait(&status) < 0) {
        err(1, "failed wait");
    }
    if (!WIFEXITED(status)) {
        err(2, "child exit");
    }
    if (WEXITSTATUS(status) != 0) {
        err(3, "child dindt exit with 0");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        errx(4, "need file");
    }

    if (mkfifo("foobar", 0666) < 0) {
        err(5, "failed mkfifo");
    }

    int pid = fork();
    if (pid < 0) {
        err(6, "failed fork");
    }

    if (pid == 0) {
        int fd = open("foobar", O_WRONLY);
        if (fd < 0) {
            err(7, "failed opening fifo");
        }

        if (dup2(fd, 1) < 0) {
            err(8, "failed dup2");
        }

        if (execlp("cat", "cat", argv[1], (char*) NULL) < 0) {
            err(9, "failed exec");
        }
    }

    asserted_wait();
    exit(0);
}
