#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

typedef struct {
    uint32_t magic;
    uint32_t count;
} header_d;

typedef struct {
    uint32_t magic1;
    uint16_t magic2;
    uint16_t reserved;
    uint64_t count;
} header_c;

typedef struct {
    uint16_t type;
    uint16_t reserved[3];
    uint32_t offset1;
    uint32_t offset2;
} pair_c;

void assertReserved(pair_c p){
        for(int i = 0; i < 3; i++){
                if(p.reserved[i] != 0){
                        errx(20,"reserved values in complect can only be 0");
                }
        }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        errx(1, "need 2 files as args");
    }

    int data = open(argv[1], O_RDWR);
    int com = open(argv[2], O_RDONLY);

    if (data < 0 || com < 0) {
        err(2, "failed opening files");
    }

    struct stat sd, sc;
    if (fstat(data, &sd) < 0 || fstat(com, &sc) < 0) {
        err(17, "failed fstat");
    }

    uint32_t sized = sd.st_size - sizeof(header_d);
    if (sized % sizeof(uint64_t) != 0) {
        errx(18, "invalid file format");
    }

    uint32_t sizec = sc.st_size - sizeof(header_c);
    if (sizec % sizeof(pair_c) != 0) {
        errx(19, "invalid file format");
    }

    // validating headers
    // data.bin reading header
    header_d hd;
    if (read(data, &hd, sizeof(header_d)) != sizeof(header_d)) {
        err(3, "failed reading header from data");
    }

    if (hd.magic != 0x21796F4A) {
        errx(4, "invalid data file");
    }

    // comparator.bin reading header
    header_c cd;
    if (read(com, &cd, sizeof(cd)) != sizeof(cd)) {
        err(5, "failed reading header from comparator");
    }

    if (cd.magic1 != 0xAFBC7A37 || cd.magic2 !=  0x1C27) {
        errx(6, "invalid comparator file");
    }

    for (int i = 0; i < cd.count; i++) {
        pair_c pair;
        if (read(com, &pair, sizeof(pair_c)) != sizeof(pair_c)) {
            err(7, "failed read of pair");
        }

        assertReserved(pair);

        if (pair.type != 0 && pair.type != 1) { continue; }

        uint64_t n1, n2;
        if (lseek(data, sizeof(header_d) + pair.offset1 * sizeof(uint64_t), SEEK_SET) < 0) {
            err(8, "failed lseek");
        } else {
            if (read(data, &n1, sizeof(uint64_t)) != sizeof(uint64_t)) {
                err(9, "couldnt read");
            }
        }

        if (lseek(data, sizeof(header_d) + pair.offset2 * sizeof(uint64_t), SEEK_SET) < 0) {
            err(10, "failed lseek");
        } else {
            if (read(data, &n2, sizeof(uint64_t)) != sizeof(uint64_t)) {
                err(11, "couldnt read");
            }
        }

        if ((pair.type == 1 && n1 < n2) || (pair.type == 0 && n1 > n2)) {
            if (lseek(data, sizeof(header_d) + pair.offset1 * sizeof(uint64_t), SEEK_SET) < 0) {
                err(12, "failed lseek");
            }
            if (write(data, &n2, sizeof(uint64_t)) != sizeof(uint64_t)) {
                err(13, "failed write");
            }

            if (lseek(data, sizeof(header_d) + pair.offset2 * sizeof(uint64_t), SEEK_SET) < 0) {
                err(14, "failed lseek");
            }
            if (write(data, &n1, sizeof(uint64_t)) != sizeof(uint64_t)) {
                err(15, "failed write");
            }
        }
    }

    close(data);
    close(com);

    return 0;
}
