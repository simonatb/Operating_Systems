#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/stat.h>
#include <sys/wait.h>

const uint8_t BYTES[4] = {0X00, 0x55, 0x7D, 0xFF};

const uint8_t ESCAPE = 0x7D;

const uint8_t WAIT = 0x01;

void asserted_wait(void) {
    int status;
    if (wait(&status) < 0) {
        err(12, "cant wait");
    }
    if (!WIFEXITED(status)) {
        errx(13, "child didnt end normally");
    }
    if (WEXITSTATUS(status) != 0) {
        errx(14, "child status not 0");
    }
}

int check_byte(uint8_t byte) {
    for (int i = 0; i < 4; i++) {
        if (byte == BYTES[i]) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        errx(1, "invali argument count");
    }

    int parent_to_child[3][2];
    int child_to_parent[3][2];

    for (int i = 0; i < 3; i++) {
        if (pipe(parent_to_child[i]) < 0 || pipe(child_to_parent[i]) < 0) {
            err(2, "failed pipe");
        }

        int pid = fork();
        if (pid < 0) {
            err(3, "failed fork");
        }

        if (pid == 0) {
            close(child_to_parent[i][0]);
            if (dup2(child_to_parent[i][1], 1) < 0) {
                err(4, "failed dup2");
            }
            close(child_to_parent[i][1]);

            close(parent_to_child[i][1]);
            if (dup2(parent_to_child[i][1], 1) < 0) {
                err(4, "failed dup2");
            }
            close(parent_to_child[i][1]);

            execlp(argv[i + 1], argv[i + 1], (char*)NULL);
            err(5, "failed exec %s", argv[1]);
        }
        close(parent_to_child[i][0]);
        close(child_to_parent[i][1]);
    }

    uint8_t byte;
    uint8_t prev = 0xFF;
    int hasPrev = 0;

    while(1) {
        int size = read(0, &byte, sizeof(uint8_t));
        if (size == 0) {
            break;
        }
        if (size < 0) {
            err(6, "failed reading byte");
        }
        if (size != sizeof(uint8_t)) {
            errx(7, "corrupted input");
        }

        uint8_t toSend = byte;
        int esc;

        if (check_byte(byte) == 1) {
            toSend = byte ^ 0x20;
        }

        if (hasPrev && prev == byte) {
            esc = 1;
        }

        prev = byte;
        hasPrev = 1;

        for (int i = 0; i < 3; i++) {
            if (esc == 1) {
                uint8_t buff[2] = {ESCAPE, toSend};
                if (write(parent_to_child[i][1], buff, 2) != 2) {
                    err(8, "failed writing to child");
                }
            } else {
                if (write(parent_to_child[i][1], &toSend, 1) != 1) {
                    err(9, "failed writing byte");
                }
            }
        }

        for (int i = 0; i < 3; i++) {
            uint8_t toReceive;

            if (read(child_to_parent[i][0], &toReceive, 1) != 1) {
                err(10, "failed receiving |");
            }

            if (toReceive != WAIT) {
                errx(11, "corrupted byte from child");
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        close(parent_to_child[i][1]);
        close(child_to_parent[i][0]);
    }

    for (int i = 0; i < 3; i++) {
        asserted_wait();
    }

    return 0;
}
