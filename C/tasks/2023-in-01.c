#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

const char *L[] = {"tic ", "tac ", "toe\n"};

void extraRead(int from, int read, int limit) {
    int curr;
    while (read(from, &curr, sizeof(curr)) == sizeof(curr)) {
        if (curr < limit) {
            curr++;
            if (write(to, &curr, sizeof(curr)) != sizeof(curr)) {
                err(6, "failed writing to next pipe");
            }
            if (write(1, L[(curr - 1) % 3], 4) != 4) {
                err(7, "err writing word");
            }
        } else {
            curr++;
            if (write(to, &curr, sizeof(curr)) != sizeof(curr)) {
                err(8, "err writing last");
            }
            close(from);
            close(to);
            exit(0);
        }
    }
}

int main (int argc, char* argv[]) {
    if (argc != 3) {
        errx(1, "need 2 nums");
    }

    int NC = strtol(argv[1], NULL, 10);
    int WC = strtol(argv[2], NULL, 10);
    if (NC < 1 || NC > 7 || WC < 1 || WC > 35) {
        errx(2, "number is not in the margins");
    }

    int count = 0;
    int pfds[8][2];
    for (int i = 0; i <= NC; i++) {
        if (pipe(pfds[i]) < 0) {
            err(3, "failed pipe");
        }
    }

    for (int i = 0; i < NC; i++) {
        int pid = fork();
        if (pid < 0) {
            err(4, "failed fork");
        }
        if (pid == 0) {
            for (int j = 0; j <= NC; j++) {
                if (j == i) {
                    close(pfds[j][1]); // close the end for writing to the prev child
                }
                else if (j == i + 1) {
                    close(pfds[j][0]); // close the end for reading to the next child
                }
                else {
                    close(pfds[j][0]); // the rest are closed
                    close(pfds[i][1]);
                }
            }
            extraRead(pfds[i][0], pfds[i+1][1], WC);
            exit(0);
        }
    }


    for (int j = 0; j <= NC; j++) {
        if (j == NC) {
            close(pfds[j][1]); // the pipe of the father we close the writing end
        }
        else if (j == 0) {
            close(pfds[j][0]); // close the reading end of the first pipe
        }
        else {
            close(pfds[j][0]); // these pipes are not of the parent
            close(pfds[i][1]);
        }
    }

    if (write(pfds[0][1], &count, sizeof(count)) != sizeof(count)) {
        err(5, "writing to the first pipe");
    }
    extraRead(pfds[NC][0], pfds[0][1], WC);

}
