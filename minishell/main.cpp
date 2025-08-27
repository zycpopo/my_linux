#include "minishell.h"
#include <stdio.h>
#include <string.h>

#define CMD_MAX_LEN 1024 // 命令字符串最大长度

int main() {
    char cmd_str[CMD_MAX_LEN] = {0};

    // Shell主循环（持续接收命令）
    while (1) {
        InitGlobal(); // 初始化全局变量（清空参数、重定向状态）
        PrintCommandPrompt(); // 打印提示符
        // 读取用户输入（空输入则重新循环）
        if (!GetCommandString(cmd_str, sizeof(cmd_str))) {
            memset(cmd_str, 0, sizeof(cmd_str));
            continue;
        }
        // 处理重定向（解析命令中的>、>>、<）
        CheckRedir(cmd_str);
        // 解析命令为参数数组（失败则重新循环）
        if (!ParseCommandString(cmd_str)) {
            memset(cmd_str, 0, sizeof(cmd_str));
            continue;
        }
        // 先执行内建命令：内建命令返回true，外部命令返回false
        if (!BuiltInCommandExec()) {
            ForkAndExec(); // 执行外部命令（创建子进程）
        }
        // 清空命令缓冲区
        memset(cmd_str, 0, sizeof(cmd_str));
    }

    return 0;
}