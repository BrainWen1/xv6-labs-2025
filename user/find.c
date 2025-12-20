#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

static int use_exec = 0;
static char *argv_exec[MAXARG] = {0};
static int count = 0;

void fmt_pathname(char *buf, char *path, char *name);
void find(char *path, char *file);
void cmd(char *buf);

int main(int argc, char *argv[]) {

    if (argc < 3) {
        fprintf(2, "Usage: find [directory] [file]\n");
        exit(1);
    }

    // 处理 -exec 选项
    if (argc > 3) {
        if (strcmp(argv[3], "-exec") != 0) {
            fprintf(2, "Usage: find [directory] [file] -exec [arguments...]\n");
            exit(1);
        }

        use_exec = 1;
        count = argc - 4;

        if (count == 0) {
            fprintf(2, "Usage: find [directory] [file] -exec [arguments...]\n");
            exit(1);
        }

        for (int i = 0; i < count; ++i) {
            argv_exec[i] = argv[i + 4];
        }
    }

    find(argv[1], argv[2]);

    exit(0);
}

void find(char *path, char *file) {
    char buf[512] = {0};
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, O_RDONLY)) < 0) { // 打开当前目录
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) { // 获取当前目录的信息
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type != T_DIR) { // 如果当前不是目录，报错并返回
        fprintf(2, "find: %s is not a directory\n", path);
        close(fd);
        return;
    }
 
    while (read(fd, &de, sizeof(de)) == sizeof(de)) { // 读取当前目录的每一个条目
        if (de.inum == 0) {
            continue;
        }

        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) { // 跳过 . 和 .. 避免无限递归
            continue;
        }

        fmt_pathname(buf, path, de.name); // 负责拼接完整的路径

        if (stat(buf, &st) < 0) { // 获取当前条目的信息
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }

        switch (st.type) { // 分情况处理当前条目
            case T_DEVICE:
            case T_FILE:
                if (strcmp(de.name, file) == 0) { // 如果是普通文件并且是待查找的文件名
                	if (use_exec == 0) {
                        printf("%s\n", buf);
                    }
                    else {
                        cmd(buf);
                    }
                }
                break;
            case T_DIR:
                find(buf, file); // 如果是目录则递归查找
                break;
            default:
                break;
        }
    }
    close(fd);
}

void fmt_pathname(char *buf, char *path, char *name) { // 辅助函数，负责拼接完整的路径
    strcpy(buf, path); // 从头开始覆盖原数据，并且自动在末尾添'\0'
    char *p = buf + strlen(buf);
    *p++ = '/';
    strcpy(p, name);
}

void cmd(char *buf) {
    int pid = fork();

    if (pid < 0) {
        fprintf(2, "find: fork %s failed\n", buf);
    }

    if (pid == 0) { // 子进程
        char *new_argv[MAXARG] = {0};

        int i = 0;
        for (i = 0; i < count; ++i) {
            new_argv[i] = argv_exec[i];
        }

        new_argv[i++] = buf;
        new_argv[i] = 0;
        exec(new_argv[0], new_argv); // exec调用
        
        fprintf(2, "find: exec %s failed\n", new_argv[0]);
        exit(1);
    }
    
    else { // 父进程
        int status;
        wait(&status); // 等待子进程终止
    }
}
