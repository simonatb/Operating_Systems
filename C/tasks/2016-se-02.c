#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

typedef struct {
    uint32_t x;
    uint32_t y;
} pair_t;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        errx(1, "need 3 files")
    }

    int f1 = open(argv[1], O_RDONLY);
    if (f1 < 0) {
        err(2, "err f1 opening");
    }

    int f2 = open(argv[2], O_RDONLY);
    if (f2 < 0) {
        err(3, "err f2 opening");
    }

    int f3 = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR);
    if (f3 < 0) {
        err(4, "err f3 opening");
    }

    struct stat s1, s2;
    if (fstat(f1, &s1) < 0 || fstat(f2, &s2) < 0) {
        err(5, "err fstat");
    }

    if (s1.st_size % sizeof(pair_t) != 0) {
        errx(6, "invalid file f1");
    }

    if (s2.st_size % sizeof(uint32_t) != 0) {
        errx(7, "invalid file f2");
    }

    pair_t pair;

    while(read(f1, &pair, sizeof(pair)) == sizeof(pair)) {
        if (lseek(f2, pair.x * sizeof(uint32_t), SEEK_SET) < 0) {
            err(8, "cant lseek");
        }

        uint32_t num;
        for (int i = 0; i < pair.y; i++) {
            if (read(f2, &num, sizeof(num)) != sizeof(num)) {
                err(9, "cant read num");
            }
            if (write(f3, &num, sizeof(num)) != sizeof(num)) {
                err(10, "cant write");
            }
        }
    }

    close(f1);
    close(f2);
    close(f3);

    return 0;
}
