#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <sys/wait.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        errx(1, "need two args");
    }

    int ifd = open(argv[1], O_RDONLY);
    if (ifd < 0) {
        err(2, "cant open input");
    }

    struct stat s;
    if (fstat(ifd, &s) < 0) {
        err(4, "fstat fail");
    }
    int size = s.st_size;

    int ofd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
    if (ofd < 0) {
        err(3, "cant open output");
    }

    uint8_t byte;

    for (int i = 0; i < size; i++) {
        if (read(ifd, &byte, sizeof(byte)) != sizeof(byte)) {
            err(5, "cant read byte");
        }
        uint16_t toSend = 0;
        for (int j = 0; j < 8; j++) {
            uint8_t mask = 1 << (7 - j);
            if (mask & byte) {
                toSend |= 1 << (15 - j*2);
            } else {
                toSend |= 1 << (14 - j*2);
            }
        }

        uint8_t high = (toSend >> 8) & 0xFF;
        uint8_t low = toSend & 0xFF;

        if (write(ofd, &high, sizeof(high)) != sizeof(high) ||
            write(ofd, &low, sizeof(low)) != sizeof(low)) {
            err(6, "failed write to output");
        }
    }

    close(ifd);
    close(ofd);
}

}
