#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <stdlib.h>

int findStart(int fd, int pos) {
    if (pos == 0) { return 0; }
    uint8_t p;
    for (int i = pos - 1; i >= 0; i--) {
        if (lseek(fd, i, SEEK_SET) < 0) {
            err(1, "failed lseek");
        }
        if (read(fd, &p, sizeof(p)) != sizeof(p)) {
            err(2, "failed reading byte");
        }
        if (p == 0) { return i; }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        errx(3, "need two args");
    }

    const char* word = argv[1];
    int dict = open(argv[2], O_RDONLY);
    if (dict < 0) {
        err(4, "failed opening file");
    }

    struct stat s;
    if (fstat(dict, &s) < 0) { err(5, "failed fstat"); }

    int size = s.st_size;

    int left = 0;
    int right = size;

    char curr[64];

    while (left < right) {
        int mid = (left + right) / 2;
        int start = findStart(dict, mid);

        if (start < 0 || lseek(dict, start + 1, SEEK_SET) < 0) {
            errx(5, "failed lseek dict");
        }

        uint8_t p;
        int len = 0;
        while (len < 63) {
            if (read(dict, &p, sizeof(p)) != sizeof(p)) {
                err(6, "failed read");
            }
            if (p == '\n') {
                break;
            }
        curr[len++] = p;
        }
        curr[len] = '\0';

        int value = strcmp(curr, word);
        if (value == 0) {
            uint8_t byte;
            while (read(dict, &byte, sizeof(byte)) == sizeof(byte)) {
                if (byte == 0) {
                    break;
                }
                if (write(1, &byte, sizeof(byte)) != sizeof(byte)) {
                    err(7, "failed write to stdoout");
                }
            }
            byte = '\n';
            if (write(1, &byte, sizeof(byte)) != sizeof(byte)) {
                err(8, "failed write end to stdout");
            }
            close(dict);
            return 0;
        } else if (value > 0) {
            right = start;
        } else {
            left = mid + 1;
        }
        }

        const char def[] = "no definition found\n";
        if (write(1, def, sizeof(def)) != strlen(def)) {
            err(9, "couldnt write no definition found");
        }

        return 0
    }
}
