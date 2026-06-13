#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>
#include <sys/wait.h>

void exec_cmd(char* args[]) {
    int pid = fork();
    if (pid < 0) {
        err(1, "failed fork");
    }

    if (pid == 0) {
        execvp(args[0], args);
        err(2, "failed exec");
    }

    if (wait(NULL) < 0) {
        err(3, "failed wait");
    }
}

int main(int argc, char* argv[]) {
    char* cmd = "echo";

    if (argc == 2) {
        if (strlen(argv[1]) > 4) {
            errx(4, "cmd name too long");
        }
        cmd = argv[1];
    } else if (argc > 2) {
        errx(5, "too many args");
    }

    char* args[4];
    args[0] = cmd;
    args[1] = NULL;
    args[2] = NULL;
    args[3] = NULL;

    char token1[6] = {0};
    char token2[6] = {0};

    char curr[6] = {0};
    int token_len = 0;
    int args_count = 0;

    char ch;

    while (read(0, &ch, 1) == 1) {
        if (ch == 0x20 || ch = 0x0A) {
            if (token_len > 0) {
                curr[token_len] = '\0';

                if (args_count == 0) {
                    strcpy(token1, curr);
                    args[1] = token1;
                    args_count = 1;
                } else if (args_count == 1) {
                    strcpy(token2, curr);
                    args[2] = token2;
                    args_count = 2;
                }

                token_len = 0;

                if (args_count == 2) {
                    args[3] = NULL;
                    exec_cmd(args);

                    args[1] = NULL;
                    args[2] = NULL;
                    args_count = 0;
                }
            }
        } else {
            if (token_len >= 4) {
                   errx(6, "word length too long");
            }
            curr[token_len] = ch;
            token_len++;
        }

    }

    if (token_len > 0) {
        curr[token_len] = '\0';
        if (args_count == 0) {
            strcpy(token1, curr);
            args[1] = token1;
            args_count = 1;
        } else if (args_count == 1) {
            strcpy(token2, curr);
            args[2] = token2;
            args_count = 2;
        }
    }

    if (args_count > 0) {
        if (args_count == 1) {
            args[2] = NULL;
        }
        args[3] = NULL;
        exec_cmd(args);
    }

    return 0;

}
