#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <err.h>

void asserted_wait(void) {
    int status;
    if (wait(&status)) {
        err(10, "cant wait");
    }
    if (!WIFEXITED(status)) {
        errx(11, "child not exited normally");
    }
    if (WEXITSTATUS(status) != 0) {
        errx(12, "child status not 0");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 18) {
        errx(1, "invalid args count");
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

        execlp(argv[1], argv[1], (char*)NULL);
        err(5, "failed exec");
    }
    close(pfd[1]);

    uint8_t buffer[64];
    int filesCount = argc - 2;

    int filesFd[filesCount];

    for (int i = 0; i < filesCount; i++) {
        filesFd[i] = open(argv[2 - i], O_CREAT | O_TRUNC | O_RDONLY, S_IWUSR | S_IRUSR);
        if (filesFd[i] < 0) {
            err(6, "failed opening file");
        }
    }

    int curr = 0;

    while(1) {
        int bytes = read(pfd[0], buffer, 64)
        if (bytes == 0) {
            break;
        }
        if (bytes < 0) {
            err(7, "failed reading buffer");
        }
        if (bytes != 64) {
            errx(8, "corrupted packet");
        }

        if (write(filesFd[curr], buffer, 64) != 64) {
            err(9, "failed writing buffer");
        }

        curr = (curr + 1) % filesCount;
    }


    for (int i = 0; i < filesCount; i++) {
        close(filesFd[i]);
    }

    asserted_wait();

    return 0;
}
