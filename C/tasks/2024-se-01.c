#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        errx(1, "need 3 args");
    }

    int n = strtol(argv[2], NULL, 10);
    if (n < 0 || n > ~(uint8_t)0) {
        errx(2, "need n to be > 0 & < 255");
    }

    int rfd = open("/dev/random", O_RDONLY);
    if (rfd < 0) {
        err(3, "failed opening random");
    }

    for (int i = 0 ; i < n; i++) {
        uint16_t s;
        if (read(rfd, &s, sizeof(s)) < sizeof(s)) {
            err(4, "couldnt read s from random");
        }

        if (s < 0) {
            continue;
        }

        int size = ~(uint16_t)0;
        uint8_t buff[size];
        if (read(rfd, buff, s) != s) {
            err(5, "failed reading in buffer");
        }

        int pipefd[2];
        if (pipe(pipefd) < 0) {
            err(6, "failed pipe");
        }

        int pid = fork();
        if (pid < 0) {
            err(7, "fork failed");
        }

        if (pid == 0) {
            close(pipefd[1]);
            if (dup2(pipefd[0], 0) < 0) {
                err(8, "failed dup2");
            }
            close(pipefd[0]);

            int nullfd = open("/dev/null", O_WRONLY);
            if (nullfd < 0) {
                err(9, "failed opening null");
            }
            if (dup2(nullfd, 1) < 0) {
                err(10, "failled dup2");
            }
            if (dup2(nullfd, 2) < 0) {
                err(11, "failled dup2");
            }
            close(nullfd);

            execlp(argv[1], argv[1], (const char*) NULL);
            err(12, "failed exec child");
        }

        close(pipefd[0]);
        if (write(pipefd[1], bytes, s) != s) {
            err(13, "failed write");
        }
        close(pipefd[1]);

        int status;
        if (waitpid(pid, &status, 0) < 0) {
            err(14, "failed waitpid");
        }

        if (!WIFEXITED(status)) {
            int outfd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (outfd < 0) {
                err(15, "failed opening out file");
            }
            if (write(outfd, bytes, s) != s) {
                err(16, "failed write to outfd");
            }
            close(outfd_;
            close(rfd);
            return 42;
        }
    }
    int outfd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (outfd < 0) {
        err(17, "failed opening file");
    }
    close(outfd);
    close(rfd);
    return 0;
}
