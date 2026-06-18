#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <err.h>
#include <sys/wait.h>

int wheels[4];
int wtp[4][2];
int ptw[4][2];

int did;
int dfd[2];

void start_wheel(int i) {
    if (wtp[i][0] > 0) close(wtp[i][0]);
    if (ptw[i][1] > 0) close(ptw[i][1]);

    if (pipe(wtp[i]) < 0 || pipe(ptw[i]) < 0) {
        err(1, "failed pipe");
    }

    int pid = fork();
    if (pid < 0) {
        err(2, "failed fork");
    }

    if (pid == 0) {
        close(wtp[i][0]);
        if (dup2(wtp[i][1], 1) < 0) {
            err(3, "failed dup2");
        }
        close(wtp[i][1]);

        close(ptw[i][1]);
        if (dup2(ptw[i][0], 0) < 0) {
            err(4, "failed dup2");
        }
        close(ptw[i][0]);

        execlp("./fake_wheel", "./fake_wheel", (char*)NULL);
        err(5, "failed exec wheel");
    }
    close(wtp[i][1]);
    close(ptw[i][0]);

    wheels[i] = pid;
}

void start_driver(void) {
    if (dfd[0] > 0) close(dfd[0]);

    if (pipe(dfd) < 0) {
        err(13, "failed pipe");
    }

    did = fork();
    if (did < 0) {
        err(14, "failed fork");
    }

    if (did == 0) {
        close(dfd[0]);
        if (dup2(dfd[1], 1) < 0) {
            err(15, "failed dup2");
        }
        close(dfd[1]);

        execlp("./fake_driver", "./fake_driver", (char*)NULL);
        err(16, "failed driver exec");
    }
    close(dfd[1]);
}

int main(void) {
    start_driver();
    for (int i = 0; i < 4; i++) {
        start_wheel(i);
    }

    uint8_t packet[16];
    int power = 0;

    while (1) {
        if (read(dfd[0], packet, 16) != 16) {
            int status;
            waitpid(did, &status, 0);
            start_driver();
            continue;
        }

        uint16_t gas = (packet[8] << 8) | packet[9];
        uint16_t velocity = 0;
        uint32_t sum = 0;
        int skip = 0;

        for (int i = 0; i < 4; i++) {
            if (read(wtp[i][0], packet, 16) != 16) {
                skip = 1;
                break;
            }

            velocity = (packet[2] << 8) | packet[3];
            sum += velocity;
        }

        if (skip) { goto handle_dead; }

        uint16_t avg = sum / 4;

        if (avg < gas) {
            power++;
        }
        else if (avg > gas) {
            power--;
        }

        uint8_t toSend[16] = {0};
        toSend[2] = (power >> 8) & 0xFF;
        toSend[3] = power & 0xFF;

        for (int i = 0; i < 4; i++) {
            write(ptw[i][1], toSend, 16);
        }

        handle_dead: ;
        int status;
        int dead = waitpid(-1, &status, WNOHANG);

        if (dead > 0) {
            if (dead == did) {
                start_driver();
            }
            else {
                for (int i = 0; i < 4; i++) {
                    if (wheels[i] == dead) {
                        close(wtp[i][0]);
                        close(ptw[i][1]);

                        start_wheel(i);
                    }
                }
            }
        }

    return 0;
}
