#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

bool compare(const char* str1, const char* str2, uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
        if (str2[i] == '\0') {
            return false;
        }
        if (str1[i] != str2[i]) {
            return false;
        }
    }

    if (str2[n] != '\0') {
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        errx(1, "need 3 args");
    }
    int indexes = open(argv[1], O_RDONLY);
    int values = open(argv[2], O_RDONLY);
    if (indexes < 0 || values < 0) {
        err(2, "cant open files");
    }

    int index = 0;
    bool found = false;
    uint8_t type = 0;

    while (1) {
        uint8_t byte;

        ssize_t read_bytes = read(indexes, &byte, sizeof(uint8_t));
        if (read_bytes = 0) {
            break;
        }
        if (read_bytes < 0) {
            err(3, "couldnt read indexes");
        }

        type = byte >> 7;
        uint8_t n = byte & 0x7F;

        char name[256];
        if (read(indexes, name, n) != n) {
            err(4, "cant read name");
        }
        name[n] = '\0';

        if (compare(argv[3], name, n)) {
            found = true;
            break;
        }
        index++;
    }

    if (found) {
        uint32_t value;
        if (lseek(values, index * sizeof(uint32_t), SEEK_SET) < 0) {
            err(5, "failed lseek");
        }
        if (read(values, &value, sizeof(uint32_t)) != sizeof(uint32_t)) {
            err(6, "couldnt read value");
        }
        if (type == (uint8_t)1) {
            float num = *(float*)&value;
            printf("%.3f\n", num);
        } else {
            printf("%d\n", (uint32_t)value);
        }
    } else {
        errx(7, "didnt find value");
    }

    close(indexes);
    close(values);
    return 0;

}
