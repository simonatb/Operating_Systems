#include <fcntl.h>
#include <stdint.h>
#include <sys/wait.h>
#include <stdio.h>
#include <err.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>

typedef struct {
    uint64_t timestamp;
    uint8_t length;
    char text[256];
} replica_t;

typedef struct {
    int fd;
    char role_name[256];
    replica_t curr_rep;
    bool is_eof;
} file_t;

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 21) {
        errx(1, "need 1 to 20 files");
    }

    file_t files[20];
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            err(2, "cant open file");
        }
        len++;
        files[i - 1].fd = fd;
        files[i - 1].is_eof = false;

        uint64_t id;
        if (read(fd, &id, sizeof(id)) != sizeof(id)) {
            err(3, "couldnt read header id");
        }
        if (id != (uint64_t)133742) {
            errx(4, "not a correct file");
        }

        uint8_t n;
        if (read(fd, &n, sizeof(n)) != sizeof(n)) {
            err(5, "couldnt read n header");
        }

        if (read(fd, files[i - 1].role_name, n) != n) {
            err(6, "couldnt read name");
        }
        files[i - 1].role_name[n] = '\0';

        if (read(fd, &files[i - 1].curr_rep.timestamp, sizeof(uint64_t)) == sizeof(uint64_t) &&
            read(fd, &files[i - 1].curr_rep.length, sizeof(uint8_t)) == sizeof(uint8_t)) {

            uint8_t len = files[i - 1].curr_rep.length;
            if (read(fd, files[i - 1].curr_rep.text, len) != len) {
                err(7, "corrupted file");
            }
            files[i - 1].curr_rep.text[len] = '\0';
        } else {
            close(fd);
            files[i - 1].is_eof = true;
        }
    }

    while (1) {
        int min_idx = -1;
        uint64_t min_time = UINT64_MAX;

        for (int i = 1; i < argc; i++) {
            if (!files[i - 1].is_eof) {
                if (files[i - 1].curr_rep.timestamp < min_time) {
                    min_idx = i - 1;
                    min_time = files[i - 1].curr_rep.timestamp;
                }
            }
        }

        if (min_idx == -1) {
            break;
        }

        printf("%s: %s\n", files[min_idx].role_name, files[min_idx].curr_rep.text);

        int fd = files[min_idx].fd;

        if (read(fd, &files[min_idx].curr_rep.timestamp, sizeof(uint64_t)) == sizeof(uint64_t) &&
            read(fd, &files[min_idx].curr_rep.length, sizeof(uint8_t)) == sizeof(uint8_t)) {

            uint8_t len = files[min_idx].curr_rep.length;
            if (read(fd, files[min_idx].curr_rep.text, len) != len) {
                err(8, "corrupted file");
            }
            files[min_idx].curr_rep.text[len] = '\0';
        } else {
            close(fd);
            files[min_idx].is_eof = true;
        }
    }

    return 0;
}
