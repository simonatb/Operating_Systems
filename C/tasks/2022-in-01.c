#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <sys/wait.h>

void waitProcess(void){
        int status;
        if(wait(&status) < 0){
                err(9,"can't wait");
        }
        if(!WIFEXITED(status)){
                err(8,"didn't exit");
        }
        if(WEXITSTATUS(status) != 0){
                err(7,"didn't exit with 0");
        }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        err(1, "need two number args");
    }

    int n = strtol(argv[1], NULL, 10);
    int d = strtol(argv[2], NULL, 10);\

    int pfd[2];
    int cfd[2];

    if (pipe(pfd) < 0 || pipe(cfd) < 0) {
        err(2, "pipe fail");
    }

    pid_t pid = fork();
    if (pid < 0) {
        err(3, "fork fail");
    }

    if (pid == 0) {
        close(pfd[1]);
        close(cfd[0]);

        char s;
        for (int i = 0; i < n; i++) {
            if (read(pfd[0], &s, 1) != 1) {
                err(4, "cant write inpfd");
            }
            if (write(1, "DONG\n", 5) != 5) {
                err(5, "cant write dong");
            }
            if (write(cfd[1], "s", 1) != 1) {
                err(6, "xhild write error");
            }
        }

        close(pfd[0]);
        close(cfd[1]);
    } else {
        close(pfd[0]);
        close(cfd[1]);

        char s;
        for (int i = 0; i < n; i++) {
            if (write(1, "DING\n", 5) != 5) {
                err(7, "cant write parent");
            }
            if (write(pfd[1], "s", 1) != 1) {
                err(8, "cant write pfd");
            }
            if (read(cfd[0], "s", 1) != 1) {
                err(9, "xhild write error");
            }

            sleep(d);
        }

        close(pfd[1]);
        close(cfd[0]);
        waitProcess();
    }
    return 0;
}
