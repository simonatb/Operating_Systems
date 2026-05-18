
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <err.h>

const char* str = ".hash";

bool isHash(const char* file, int len) {
    if (len <= 5) {
        return false;
    }
    for (int i = 1; i <= 5; i++) {
        if (file[len - i] != str[5 - i]) {
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        err(1, "need one arg");
    }

    int pfd[2];
    if (pipe(pfd) < 0) {
        err(2, "failed pipe");
    }

    int pid = fork();
    if (pid < 0) {
        err(3, "failed fork");
    }

    if (pid == 0) {
        close(pfd[0]);
        if (dup2(pfd[1], 1) < 0) {
            err(4, "failed dup2");
        }
        close(pfd[1]);

        execlp("find", "find", argv[1], "-type", "f", (char*)NULL);
        err(5, "failed find execlp");
    }
        close(pfd[1]);

        char buffer[4096];
        char line[4096];
        int curr = 0;
        ssize_t bytes;

        while ((bytes = read(pfd[0], buffer, sizeof(buffer))) > 0) {
            for (int i = 0; i < bytes; i++) {
                if (buffer[i] != '\n') {
                    line[curr++] = buffer[i];
                } else {
                    line[curr] = '\0';
                    if (!isHash(line, curr)) {
                        int proc = fork();
                        if (proc < 0) {
                            err(6, "failed fork for md5sum");
                        }

                        if (proc == 0) {
                            close(pfd[0]);
                            char filename[4096];
                            for (int j = 0; j < curr; j++) {
                                filename[j] = line[j];
                            }

                            filename[curr] = '.';
                            filename[curr + 1] = 'h';
                            filename[curr + 2] = 'a';
                            filename[curr + 3] = 's';
                            filename[curr + 4] = 'h';
                            filename[curr + 5] = '\0';

                            int out = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            if (out < 0) {
                                err(7, "failed open");
                            }

                            dup2(out, 1);
                            close(out);

                            execlp("md5sum", "md5sum", line, (char*)NULL);
                            err(8, "failed md5sum exec");

                        }
                    }
                    curr = 0;
                }

            }
        }
        close(pfd[0]);
        while (wait(NULL) > 0);

        return 0;

}
