#include <unistd.h>
#include <err.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>

typedef struct {
    uint16_t offset;
    uint8_t byte1;
    uint8_t byte2;
} patch_t;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        errx(1, "need 3 args");
    }

    int fd1 = open(argv[1], O_RDONLY);
    if (fd1 < 0) err(2, "can't open f1");

    int fd2 = open(argv[2], O_RDONLY);
    if (fd2 < 0) err(3, "can't open f2");

    int fd3 = open(argv[3], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd3 < 0) err(4, "can't open patch file");

    struct stat s1, s2;
    if (fstat(f1, %s1) < 0 || fstat(f2, &s2) < 0) err(5, "can't fstat");

    if (s1.st_size != s2.st_size) errx(6, "different size files");

    uint8_t b1, b2;
    uint16_t offset = 0;
    int r1, r2;

    while ((r1 = read(f1, &b1, sizeof(b1))) == sizeof(b1) && (r2 = read(f2, &b2, sizeof(b2))) == sizeof(b2)) {
        if (b1 != b2) {
            patch_t p;
            p.offset = 0;
            p.byte1 = b1;
            p.byte2 = b2;
            if (write(fd3, &p, sizeof(p)) != sizeof(p)) {
                err(7, "can't write");
            }
        }
        offset++;
    }

    if (r1 < 0 || r2 < 0) {
        err(8, "final read fail");
    }

    close(fd1);
    close(fd2);
    close(fd3);

    return 0;
}
