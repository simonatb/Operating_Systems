#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>

void read_func(char* name, int* lines, int lines_count) {
    int fd;
    if (strcmp(name, "-") == 0) {
        fd = 0;
    } else {
        fd = open(name, O_RDONLY);
    }

    char ch;
    char buffer[4096];
    int idx = 0;

     while (read(fd, &ch, sizeof(ch)) == sizeof(ch)) {
        buffer[idx++] = ch;
        if (ch == '\n' || idx >= 4095) {
            buffer[idx] = '\n';

            if (lines) {
                dprintf(1, "%d ", lines_count);
                lines_count++;
            }

            if (write(1, buffer, idx) != idx) {
                err(1, "failed writing buffer to stdout");
            }
            idx = 0;
        }
    }
}

int main(int argc, char* argv[]) {
    int lines = 0;
    if (strcmp(argv[1], "-n") == 0) {
        lines = 1;
    }

    int lines_count = 1;

    if (argc == 1) {
        read_func("-", lines, lines_count);
        return 0;
    }

    for (int i = 1 + lines; i < argc; i++) {
        read_func(argv[i], lines, lines_count);
    }

    return 0;
}
