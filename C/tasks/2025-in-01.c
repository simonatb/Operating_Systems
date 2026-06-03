#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <err.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

#define MAX_RAM 512
#define MAX_REGS 32
#define FN_LEN 8
#define IN_SIZE 4

typedef struct {
    uint16_t ram_size;
    uint16_t register_count;
    char filename[FN_LEN];
} processor;

typedef struct {
    uint8_t opcode;
    uint8_t op1;
    uint8_t op2;
    uint8_t op3;
} instruction;

void execute(const processor* p) {
    uint8_t reg[p->register_count] = {0};
    uint8_t ram[p->ram_size] = {0};

    int fd = open(p->filename, O_RDWR);
    if (fd < 0) {
        err(5, "couldn't open %s", p->filename);
    }

    if (read(fd, reg, p->register_count) != p->register_count) {
        err(6, "couldn't read reg vals");
    }

    if (read(fd, ram, p->ram_size) != p->ram_size) {
        err(7, "couldn't read ram");
    }

    off_t st = lseek(fd, 0, SEEK_CUR);
    instruction i;

    while (read(fd, &i, IN_SIZE) == IN_SIZE) {
        switch (i.opcode) {
            case 0:
                reg[i.op1] = reg[i.op2] & reg[i.op3];
                break;
            case 1:
                reg[i.op1] = reg[i.op2] | reg[i.op3];
                break;
            case 2:
                reg[i.op1] = reg[i.op2] + reg[i.op3];
                break;
            case 3:
                reg[i.op1] = reg[i.op2] * reg[i.op3];
                break;
            case 4:
                reg[i.op1] = reg[i.op2] ^ reg[i.op3];
                break;
            case 5:
                if (write(1, &reg[i.op1], 1) != 1) {
                    err(8, "failed writing to out");
                }
                break;
            case 6:
                sleep(reg[i.op1]);
                break;
            case 7:
                reg[i.op1] = ram[reg[i.op2]];
                break;
            case 8:
                ram[reg[i.op2]] = reg[i.op1];
                break;
            case 9:
                if (reg[i.op1] != reg[i.op2]) {
                    off_t offset = st + i.op3 * IN_SIZE;
                    if (lseek(fd, offset, SEEK_SET) == -1) {
                        err(9, "failed lseek");
                    }
                }
                    break;
            case 10:
                reg[i.op1] = op2;
                break;
            case 11:
                ram[reg[i.op1]] = op2;
                break;
            default:
                errx(10, "invalid opcode");
        }
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        err(11, "failed lseek");
    }
    if (write(fd, reg, p->register_count) != p->register_count) {
        err(12, "failed writing new regs");
    }
    if (write(fd, ram, p->ram_size) != p->ram_size) {
        err(13, "failed writing new ram");
    }

    close(fd);
    exit(0);
}

int main(int argc. char* argv[]) {
    if (argc != 2) {
        errx(1, "need one file");
    }

    int fd = open(argv[1], O_RDONLY);
    if (rd < 0) {
        err(2, "couldn't open %s", argv[1]);
    }

    processor p;
    while (read(fd, &p, sizeof(p)) == sizeof(p)) {
        if (p.ram_size > MAX_RAM || p.register_count > MAX_REGS) {
            errx(3, "invalid file format");
        }

        int pid = fork();
        if (pid < 0) {
            err(4, "failed fork");
        }

        if (pid == 0) {
            close(fd);
            execute(&p);
        }
    }

    close(fd);

    while (wait(NULL) > 0);

    return 0;
}
