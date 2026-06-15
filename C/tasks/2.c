#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <err.h>
#include <sys/wait.h>

#define ESC 0x7D
#define SER 0x55
#define QUE 0x3F

int compare(uint8_t a, uint8_t b, uint8_t c) {
    if (a == b) return a;
    if (c == b) return b;
    if (a == c) return c;

    return -1;
}

int read_func(int fd, uint8_t* byte) {
    uint8_t curr;
    while (read(fd, &curr, 1) == 1) {
        if (curr == ESC) {
            uint8_t next ;
            if (read(fd, &next, 1) != 1) return -1;
            *byte = next ^ 0x20;
            return 1;
        }
        if (curr == SER) {
            continue;
        }

        *byte = curr;
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        errx(1, "invalid arguments count");
    }

    int pc[3][2];
    int cp[3][2];

    for (int i = 0; i < 3; i++) {
        if (pipe(pc[i]) < 0 || pipe(cp[i]) < 0) {
            err(2, "failed pipe");
        }

        int pid = fork();
        if (pid == 0) {
            close(pc[i][1]);
            if (dup2(pc[i][0], 0) < 0) {
                err(3, "failed dup2");
            }
            close(pc[i][0]);

            close(cp[i][0]);
            if (dup2(cp[i][1], 1) < 0) {
                err(3, "failed dup2");
            }
            close(cp[i][1]);

            execlp(argv[i + 1], argv[i + 1], (char*)NULL);
            err(4, "failed exec");
        }
        close(pc[i][0]);
        close(cp[i][1]);
    }

    while (1) {
        uint8_t b[3];
        int status[3];

        for (int i = 0; i < 3; i++) {
            status[i] = read_func(cp[i][0], &b[i]);
        }

        if (status[0] < 0 || status[1] < 0 || status[2] < 0) {
            errx(7, "Corrupted stream from child process");
        }

        if (status[0] == 0 || status[1] == 0 || status[2] == 0) {
            break;
        }

        uint8_t final;
        int res = compare(b[0], b[1], b[2]);

        if (res != -1) {
            final = (uint8_t)res;
        } else {
            final = QUE;
        }

        if (write(1, &final, 1) != 1) {
            err(6, "failed writing to stdout");
        }
    }

    for (int i = 0; i < 3; i++) {
        close(pc[i][1]);
    }

    while (wait(NULL) > 0);

    return 0;

}
