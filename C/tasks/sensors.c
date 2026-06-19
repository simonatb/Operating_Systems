#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <err.h>

int pids[3];
int stp[3][2];
int pts[3][2];

int start_sensor(int i) {
    if (stp[i][0] > 0) close(stp[i][0]);
    if (pts[i][1] > 0) close(pts[i][1]);

    int pid = fork();
    if (pid < 0) {
        err(1, "failed fork");
    }

    if (pid == 0) {
        close(stp[i][0]);
        if (dup2(stp[i][1], 1) < 0) {
            err(2, "failed dup2");
        }
        close(stp[i][1]);

        close(pts[i][1]);
        if (dup2(pts[i][0], 0) < 0) {
            err(3, "failed dup2");
        }
        close(pts[i][0]);

        uint8_t package[4];
        while (1) {
            uint16_t fake = 20 + i * 20;

            package[0] = i;
            package[1] = (fake >> 8) & 0xFF;
            package[2] = fake & 0xFF;
            package[3] = package[0] ^ package[1] ^ package[2];

            write(1, package, 4);

            uint8_t in[4];
            if (read(0, in, 4) != 4) break;

            sleep(2);
        }
        exit(0);
    }
    close(stp[i][1]);
    close(pts[i][0]);

    pids[i] = pid;
}

int main(void) {
    for (int i = 0; i < 3; i++) {
        start_sensor(i);
    }

    uint8_t buffer[4];

    int fd = open("weather.log", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0) {
        err(4, "failed opening weather.log");
    }

    while (1) {
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

        int skip = 0;
        uint16_t values[3] = {0 ,0 ,0};

        for (int i = 0; i < 3; i++) {
            if (read(stp[i][0], buffer, 4) != 4) {
                skip = 1;
                break;
            }

            uint8_t id = buffer[0];
            uint16_t value = ((uint16_t)buffer[1] << 8) | buffer[2];
            uint8_t check = buffer[3];

            if (check == (buffer[0] ^ buffer[1] ^ buffer[2])) {
                values[i] = value;

                char line[64];
                int len = snprintf(line, sizeof(line), "sensor %d: %d\n", id, value);
                write(fd, line, len);
            }
        }

        if (skip) { continue; }

        uint16_t callibration = (values[0] + values[1] + values[2]) / 3;

        uint8_t response[4];
        response[0] = 99;
        response[1] = (callibration >> 8) & 0xFF;
        response[2] = callibration & 0xFF;
        response[3] = response[0] ^ response[1] ^ response[2];

        for (int i = 0; i < 3; i++) {
            write(pts[i][1], response, 4);
        }
    }

    close(fd);
    return 0;
}
