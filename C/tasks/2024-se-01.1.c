#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <err.h>
#include <sys/stat.h>

#define MAX_DATA 504

typedef struct {
    uint64_t index;
    uint8_t user_data[MAX_DATA];
} pair;

int isInFile(int fd, uint64_t ind) {
    if (lseek(fd, 0, SEEK_SET) < 0) {
        err(9, "failed lseek");
    }

    uint64_t byte = 0;
    while (read(fd, &byte, sizeof(byte)) == sizeof(byte)) {
        if (byte == ind) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        errx(1, "need one file aarg");
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        err(2, "failed opening db");
    }

    struct stat s;
    if (fstat(fd, &s) < 0) {
        err(3, "failed fstat");
    }

    int size = s.st_size;
    if (size % sizeof(pair) != 0) {
        errx(4, "file size is invalid");
    }
    size /= sizeof(pair);

    char temp_template[] = "/tmp/db_XXXXXX";
    int temp = mkstemp(temp_template);
    if (temp < 0) {
        err(5, "failed creating temp file");
    }
    unlink(temp_template);

    pair p;
    uint8_t i = 0;
    while (1) {
        if (write(temp, &i, sizeof(i)) != sizeof(i)) {
            err(6, "writing to temp failed");
        }

        if (lseek(fd, i * sizeof(pair), SEEK_SET) < 0) {
            err(7, "failed lseek");
        }

        if (read(fd, &p, sizeof(p)) != sizeof(p)) {
            err(8, "failed read");
        }

        if (p.index == 0) {
            break;
        }

        i = p.index;
    }

    uint8_t zeros[512] = {0};
    for (uint64_t i = 0; i < size; i++) {
        if (isInFile(temp, i) == 0) {
            if (lseek(fd, i * sizeof(pair), SEEK_SET) < 0) {
                err(9, "failed lseek");
            }
            if (write(fd, &zeros, 512) != 512) {
                err(10, "failed wrinig zeros");
            }
        }
        if (lseek(temp, 0, SEEK_SET) < 0) {
            err(11, "failed lseek");
        }
    }

    close(fd);
    close(temp);
    return 0;
}
