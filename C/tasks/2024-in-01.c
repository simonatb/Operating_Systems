#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <fcntl.h>
#include <stdint.h>

const int MIN = 24 * 60 * 60;
const int MAX = 10 * 60;

typedef struct {
    uint16_t magic;
    uint16_t ver;
    uint16_t cp;
    uint16_t co;
} header;

typedef struct {
    uint16_t v1;
    uint16_t v2;
    uint32_t v3;
} preamble;

typedef struct {
    uint32_t ctime;
    uint16_t opt;
    uint16_t parent_id;
    uint32_t size;
    uint32_t ssize;
} object;

void assert_header(header* h) {
    if (h->magic != 0x6963 || h->ver != 0x6e73) {
        errx(4, "invalid header file");
    }
}

int check_snap(object* o, object* obs) {
        uint16_t flag = o->opt >> 14;

    if (type != 2) {
        return 0;
    }

    if (o->parent_id == 0) {
        return 0;
    }

    uint32_t ptime = obs[o->parent_id].ctime;
    uint32_t diff = (o->ctime > ptime) ? (o->ctime - ptime) : (ptime - o->ctime);

    if (diff >= (ONE_DAY - TEN_MINUTES) && diff <= (ONE_DAY + TEN_MINUTES)) {
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        errx(1, "need one arg file");
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        err(2, "failed opening file");
    }

    header h;
    if (read(fd, &h, sizeof(header)) != sizeof(header)) {
        err(3, "fail reading header");
    }

    assert_header(&h);
    preamble p;
    for (int i = 0; i < h.cp; i++) {
        if (read(fd, &p, sizeof(p)) != sizeof(p)) {
            err(5, "failed reading preamble");
        }
    }

    object obj[h.co];
    for (int i = 0; i < h.co; i++) {
        if (read(fd, &obj[i], sizeof(obj[i])) != sizeof(obj[i])) {
            err(6, "failed reading object");
        }
    }

    int u = 0;
    int d = 0;

    for (int i = 0; i < h.co; i++) {
        if (check_snap(&obj[i], obj)) {
            u += obj[i].ssize;
            d += obj[i].size;
        }
    }

    if (d > 0) {
        double result = (double)u/d;
        printf("%f\n", result);
    } else {
        printf("no snapshots");
    }

    return 0;
}
