#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <err.h>
#include <sys/stat.h>
#include <sys/wait.h>

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
        errx(4, "need one arg");
    }

    int pid = fork();
    if (pid < 0) {
        err(5, "failed fork");
    }

    if (pid == 0) {
        int fd = open("foobar", O_RDONLY);
        if (fd < 0) {
            err(6, "failed opening foobar");
        }

        if (dup2(fd, 0) < 0) {
            err(7, "failed dup2");
        }

        if (execlp(argv[1], argv[1], (char*) NULL) < 0) {
            err(8, "failed exec");
        }
    }

    asserted_wait();
    if (unlink("foobar") < 0) {
        err(9, "failed unlink");
    }
    exit(0);
}
