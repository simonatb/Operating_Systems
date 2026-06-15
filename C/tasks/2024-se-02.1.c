#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>

typedef struct {
    uint32_t magic;
    uint32_t packet_count;
    uint64_t original_size;
} header;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        errx(1, "need two files as args");
    }

    int cfd = open(argv[1], O_RDONLY);
    if (cfd < 0) {
        err(2, "failed opening %s", argv[1]);
    }

    int ofd = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (ofd < 0) {
        err(3, "failed opening %s", argv[2]);
    }

    header h;
    if (read(cfd, &h, sizeof(h)) != sizeof(h)) {
        err(4, "failed reading header");
    }

    if (h.magic != 0x21494D46) {
        errx(5, "file not the valid format");
    }

    for (uint32_t i = 0; i < h.packet_count; i++) {
        uint8_t mask = 1;
        mask <<= 7;
        uint8_t byte;
        uint8_t n = (~mask);

        if (read(cfd, &byte, 1) != 1) {
            err(6, "failed reading byte");
        }

        n &= byte;
        mask &= byte;
        mask >>= 7;

        if (mask) {
            uint8_t b;
            if (read(cfd, &b, 1) != 1) {
                err(7, "err reading byte");
            }

            for (int i = 0; i <= n; i++) {
                if (write(ofd, &b, 1) != 1) {
                    err(8, "err writing to original file");
                }
            }
        }
        else if (mask == 0) {
            uint8_t buffer[256];
            if (read(cfd, buffer, n + 1) != n + 1 ) {
                err(9, "err reading buffer");
            }

            if (write(ofd, buffer, n + 1) != n + 1) {
                err(10, "err writing buffer to original file");
            }
        } else { errx(11, "invalid package format"); }

    }

    close(ofd);
    close(cfd);

    return 0;
}
