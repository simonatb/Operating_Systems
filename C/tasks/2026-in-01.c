#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <float.h>

typedef struct {
    uint32_t magic;
    uint32_t size;
    uint32_t files;
} header;

typedef struct {
    uint8_t min;
    uint8_t max;
} pair;

int main(int argc, char* argv[]) {
    if (argc < 4 || argc > 10) {
        errx(1, "need args between 3 and 9");
    }

    int fd = open(argv[1], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        err(2, "failed opening file");
    }
    int filesCount = argc - 2;

    int fds[filesCount];
    int done[filesCount] = {0};

    struct stat s;
    uint32_t maxSize = 0;
    for (int i = 2; i < argc; i++) {
        fds[i - 2] = open(argv[i], O_RDONLY);
        if (fds[i - 2] < 0) {
            err(3, "failed open");
        }
        if (fstat(fds[i - 2], &s) < 0) {
            err(4, "failed fstat");
        }
        if (s.st_size / sizeof(float) > maxSize) {
            maxSize = s.st_size / sizeof(float);
        }
    }

    header h;
    h.magic = 0x0F0110F0;
    h.size = maxSize;
    h.files = filesCount;

    if (write(fd, &h, sizeof(h)) != sizeof(header)) {
        err(5, "failed header write");
    }

    for (int i = 0 ;i < maxSize; i++) {
        pair p;
        float minNum = FLT_MAX;
        float maxNum = -FLT_MAX;
        float fl;
        uint8_t minIndex = 0, maxIndex = 0;

        for (int j = 0; j < filesCount; j++) {
            if (done[j] == 0) {
            uint32_t bytes;
            bytes = read(fds[i], &fl, sizeof(float);)
                if (bytes == 0) {
                    done[j] = 1;
                } else if (bytes == sizeof(float)) {
                    if (maxNum < fl) {
                        maxNum = fl;
                        maxIndex = j;
                    }
                    if (minNum > fl) {
                        minNum = fl;
                        minIndex = j;
                    }
                } else {
                    err(6, "failed read float");
                }
            }
        }
        p.min = minIndex;
        p.max = maxIndex;
        if (write(fd, &p, sizeof(p)) != sizeof(p)) {
            err(7, "failed writing a pair");
        }

    }

    for (int i = 0; i < filesCount; i++) {
        close(fds[i]);
    }
    close(fd);

    return 0;

}
