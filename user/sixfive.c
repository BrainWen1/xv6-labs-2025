#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

const char *seperator = " -\r\t\n./,";

int is_sep(char c) {
    return strchr(seperator, c) != 0;
}

void process(int fd) {
    char ch;               // 当前读取的字符
    int in_number = 0;     // 是否处于数字序列中
    int current_num = 0;   // 当前正在构建的数字
    int prev = 1;          // 前一个字符是否是分隔符（初始=1：文件开头是隐含分隔符）

    while (read(fd, &ch, 1) == 1) {

        if (ch >= '0' && ch <= '9') {  // 当前字符是数字
            if (in_number == 0) {          // 之前不在数字中 -> 尝试开始新数字
                if (prev == 1) {       // 数字开头必须是分隔符/文件开头（prev_is_sep=1）
                    in_number = 1;
                    current_num = ch - '0'; // 初始化当前数字
                }
            } else {                   // 之前已经在数字中 -> 累加数字
                current_num *= 10;
                current_num += (ch - '0');
            }

            prev = 0; // 数字不是分隔符，更新前状态
        }
        
        else {                     // 当前字符不是数字
            if (in_number == 1) {           // 之前在数字中 -> 数字结束
                // 数字结尾是当前字符：必须是分隔符/文件结尾才有效
                if (is_sep(ch)) {         // 当前字符是分隔符 -> 数字有效
                    if (current_num % 5 == 0 || current_num % 6 == 0) {
                        fprintf(1, "%d\n", current_num);
                    }
                }
                // 重置数字状态
                in_number = 0;
                current_num = 0;
            }

            prev = (is_sep(ch) ? 1 : 0);
        }
    }

    // 处理文件结尾的最后一个数字（文件结尾是隐含分隔符）
    if (in_number == 1) {
        if (current_num % 5 == 0 || current_num % 6 == 0) {
            fprintf(1, "%d\n", current_num);
        }
    }
}

int main(int argc, char *argv[]) {

    int fd;

    if (argc == 1) {
        process(0); // 标准输入
    }
    
    else { // 文本文件
        for (int i = 1; i < argc; i++) {
            fd = open(argv[i], O_RDONLY);

            if (fd < 0) {
                fprintf(2, "Error: open %s failed\n", argv[i]);
                continue; // 当前文件无法打开，处理下一个文件
            }

            process(fd);

            close(fd);
        }
    }
    
    exit(0);
}
