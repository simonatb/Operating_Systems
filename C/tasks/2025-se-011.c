#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(void) {
    if (argc != 1) {
        errx(1, "no args needed");
    }

    int dfd[2];
    if (pipe(dfd) < 0) {
        err(2, "failed pipe");
    }

    int did = fork();
    if (did == 0) {
        close(dfd[0]);
        if (dup2(dfd[1], 1) < 0) {
            err(3, "failed dup2");
        }
        close(dfd[1]);

        execl("./fake_driver", "./fake_driver", (char *)NULL);
        err(4, "failed exec of fake_driver");
    }
    close(dfd[1]);
    int d_pipe = dfd[0]; // we read gas from here

    int wheel_to_parent[4];
    int parent_to_wheel[4];

    for (int i = 0;i < 4; i++) {
        int p_w2p[2];
        int p_p2w[2];

        if (pipe(p_w2p) < 0 || pipe(p_p2w) < 0) {
            err(5, "failed pipe in wheel");
        }

        int wp = fork();
        if (wp < 0) {
            err(6, "failed wheel fork");
        }

        if (wp == 0) {
            close(p_w2p[0]);
            if (dup2(p_w2p[1], 1) < 0) {
                err(7, "failed dup2");
            }
            close(p_w2p[1]);

            close(p_p2w[1]);
            if (dup2(p_p2w[0], 0) < 0) {
                err(8, "failed dup2");
            }
            close(p_p2w[0]);

            execlp("./fake_wheel", "./fake_wheel", (char*)NULL);
            err(9, "failed exec in wheel");
        }

        close(p_w2p[1]);
        close(p_p2w[0]);

        wheel_to_parent[i] = p_w2p[0];
        parent_to_wheel[i] = p_p2w[1];

    }

    uint16_t power = 0;
    uint8_t buffer[16];

    while(1) {

        if (read(d_pipe, buffer, 16) != 16) {
            err(10, "failed read of gas");
        }

        uint16_t gas = (buffer[8] << 8) | buffer[9];

        uint32_t sum = 0;
        for (int i = 0; i < 4; i++) {
            if (read(wheel_to_parent[i], buffer, 16) != 16) {
                err(11, "failed reading speed");
            }
            uint16_t curr = (buffer[2] << 8) | packet[3];
            sum += curr;
        }
        uint16_t avg = sum / 4;

        if (avg < gas) {
            power++;
        } else if (avg > gas) {
            power--;
        }

        uint8_t toSend[16] = {0};
        toSend[2] = (power >> 8) & 0xFF;
        toSend[3] = power & 0xFF;

        for (int i = 0; i < 4; i++) {
            if (write(parent_to_wheel[i], toSend, 16) != 16) {
                err(12, "failed write power to wheel");
            }
        }
    }
}

}
