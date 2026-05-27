#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>

const int MAX = 10;

int pids[MAX];

void spawn(int i, char com[]) {
    int pid = fork();
    if (pid < 0) {
        err(2, "failed fork");
    }
    if (pid == 0) {
        execlp(com, com, (const char*) NULL);
        err(3, "failed %s", com);
    }

    pids[1] = pid;
}

int findIndex(int pid, int n) {
    for (int i = 0; i < n; i++) {
        if (pids[i] == pid) {
            return i;
        }
    }
    return -1;
}

int done(int n) {
    for (int i = 0; i < n ; i++) {
        if (pids[i] != 0) {
            return 0;
        }
    }
    return 1;
}

int main(int argc. char* argv[]) {
    if (argc < 2 || argc > 11) {
        errx(1, "need 1-10 args");
    }


    int n = argc - 1;
    for (int i = 0; i < n; i++) {
        spawn(i, argv[i + 1]);
    }

    int status;
    int pid;
    while((pid = wait(&status)) > 0) {
        int i = findIndex(pid, n);
        if (i == -1) {
            continue;
        }

        if (WIFSIGNALED(status)) {
            for (int j = 0; j < n; j++) {
                if (i != j && pids[j] != 0) {
                    kill(pids[j], SIGTERM);
                }
            }

            for (int j = 0; j < n; j++) {
                if (i != j && pids[j] != 0) {
                    waitpid(pids[j], NULL, 0);
                }
            }
            exit(i + 1);
        } else if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                pids[i] = 0;
                if (done()) {
                    exit(0);
                }
            } else {
                spawn(i, argv[i + 1]);
            }
        }

    }

    if (errno == ECHILD) {
        exit(0);
    }

    err(5, "waitpid failed");
}
