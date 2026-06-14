#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>

#define CHUNK 25000000
#define MAX 10

int compare(const void* a, const void* b) {
    uint32_t a1 = *(const uint32_t*)a;
    uint32_t a2 = *(const uint32_t*)b;
    if (a1 < a2) return -1;
    if (a1 > a2) return 1;
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        errx(1, "need one arg");
    }

    int fd_in = open(argv[1], O_RDONLY);
    if (fd_in < 0) {
        err(2, "failed opening file");
    }

    uint32_t* buff = malloc(CHUNK * sizeof(uint32_t));
    if (!buff) {
        close(fd_ind);
        err(3, "failed malloc");
    }

    int temp_fds[MAX];
    char temp_names[MAX][30];
    int chunks = 0;

    ssize_t bytes_read;
    while ((bytes_read = read(fd_in, buff, CHUNK * sizeof(uint32))) > 0) {
        if (chunks >= MAX) {
            errx(4, "too big file");
        }

        int elements = bytes_read / sizeof(uint32_t);
        qsort(buff, elements, sizeof(uint32_t), compare);

        strcpy(temp_names[chunks], "/tmp/sort_XXXXXX");
        int t_fd = mkstemp(temp_names[chunks]);
        if (t_fd < 0) {
            err(5, "failed mkstemp");
        }

        if (write(t_fd, buff, bytes_read) != bytes_read) {
            err(6, "failed writing from buffer");
        }

        if (lseek(t_fd, 0, SEEK_SET) < 0) {
            err(7, "failed lseek");
        }

        temp_fds[chunks] = t_fd;
        chunks++;
    }

    close(fd_in);
    free(buff);

    int fd_out = open(argv[1], O_WRONLY | O_TRUNC);
    if (fd_out < 0) {
        err(8, "failed opening file");
    }

    uint32_t curr_vals[MAX];
    int is_active[MAX];

    for (int i = 0; i < chunks; i++) {
        if (read(temp_fds[i], &curr_vals[i], sizeof(uint32_t)) == sizeof(uint32_t)) {
            is_active[i] = 1;
        } else {
            is_active[i] = 0;
        }
    }

    while (1) {
        int min_idx = -1;
        uint32_t min_val;

        for (int i = 0; i < chunks; i++) {
            if (is_active[i]) {
                if (min_idx == -1 || curr_vals[i] < min_val) {
                    min_val = curr_vals[i];
                    min_idx = i;
                }
            }
        }

        if (min_idx == -1) {
            break;
        }

        if (write(fd_out, &min_val, sizeof(uint32_t)) != sizeof(uint32_t)) {
            err(9, "failed writing min");
        }

        if (read(temp_fds[min_idx], &curr_vals[min_idx], sizeof(uint32_t)) != sizeof(uint32_t)) {
            is_active[min_idx] = 0;
        }
    }

    close(fd_out);

    for (int i = 0; i < chunks; i++) {
        close(temp_fds[i]);
        unlink(temp_names[i]);
    }

    return 0;
}
