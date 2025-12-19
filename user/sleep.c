#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {

    int ticks;

    if (argc != 2) { // 未传参
        fprintf(2, "Usage: Type sleep ticks\n");
        exit(1);
    }

    ticks = atoi(argv[1]);

    if (ticks <= 0) { // 数字小于等于0，或者传入参数非数字
        fprintf(2, "Error: ticks must be a positive integer\n");
        exit(1);
    }

    pause(ticks);

    exit(0);
}
