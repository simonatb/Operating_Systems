#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>
#include <sys/wait.h>

void extra_read(int i, int from, int to) {
    int curr = 0;
    while (read(from, &curr, sizeof(curr)) == sizeof(curr)) {
        if (curr != -1) {
            curr++;
            printf("[Worker %d] incremented token to %d\n", i, curr);
            if (write(to, &curr, sizeof(curr)) != sizeof(curr)) {
                err(5, "writing to next child");
            }
        } else {
            if (write(to, &curr, sizeof(curr)) != sizeof(curr)) {
                err(6, "writing next to this child");
            }
            exit(0);
        }
    }
}

void father_read(int from, int to, int limit) {
    int curr = 0;
    while (read(from, &curr, sizeof(curr)) == sizeof(curr)) {
        if (curr >= limit) {
            curr = -1;
            if (write(to, &curr, sizeof(curr)) != sizeof(curr)) {
                err(7, "father failed writing to child");
            }
        }
        else {
            curr++;
            if (write(to, &curr, sizeof(curr)) != sizeof(curr)) {
                err(8, "father failed writing to child");
            }
    }
}

void assert_args(int n, int k) {
    if (n < 2 || n > 5 || k < 10 || k > 100) {
        errx(2, "numbers are not in interval");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        errx(1, "need two args numbers");
    }

    int n = atoi(argv[1]);
    int k = atoi(argv[2]);
    assert_args(n, k);

    int pipes[n + 1][2];
    for (int i = 0; i <= n; i++) {
        if (pipe(pipes[i]) < 0) {
            err(3, "failed pipe");
        }
    }

    for (int i = 0; i < n; i++) {
        int pid = fork();
        if (pid < 0) {
            err(4, "failed fork");
        }
        if (pid == 0) {
            for (int j = 0; j <= n; j++) {
                if (i == j) {
                    close(pipes[j][1]);
                }
                else if (i + 1 == j) {
                    close(pipes[j][0]);
                }
                else {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            extra_read(i, pipes[i][0], pipes[i + 1][1]);

        }
    }

    for (int i = 0; i <= n; i++) {
        if (i == 0) {
            close(pipes[0][0]);
        }
        else if (i == n) {
            close(pipes[n][1]);
        }
        else {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }

    int token = 0;
    if (write(pipes[0][1], &curr, sizeof(curr)) != sizeof(curr)) {
        err(8, "failed starting the incrementing");
    }

    father_read(pipes[n][0], pipes[0][1], k);

    while (wait(NULL) > 0);

}
