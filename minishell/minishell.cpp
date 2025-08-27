#include "minishell.h"
#include <iostream>
#include <string.h>
#include <ctype.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>

// 全局变量定义（与头文件声明对应）
int lastcode = 0;
char pwd[1024];
int gargc = 0;
char* gargv[ARGS] = {NULL};
std::string filename;
int redir_type = NONE_REDIR;

// 调试函数
void Debug() {
    printf("hello shell!\n");
}

// 获取用户名（从环境变量USER读取）
std::string GetUsername() {
    const char* user_env = getenv("USER");
    return (user_env != NULL) ? user_env : "None";
}

// 获取主机名（优先用系统调用gethostname，兜底用环境变量）
std::string GetHostname() {
    char host_buf[256] = {0};
    if (gethostname(host_buf, sizeof(host_buf)) == 0) {
        return host_buf;
    }
    const char* host_env = getenv("HOSTNAME");
    return (host_env != NULL) ? host_env : "None";
}

// 获取当前工作目录（修复缓冲区截断，统一函数名小写p）
std::string Getpwd() {
    char temp[1024];
    if (getcwd(temp, sizeof(temp)) == NULL) {
        perror("getcwd error");
        return "Unknown";
    }

    // 避免snprintf截断：计算最大可容纳路径长度
    size_t max_path_len = sizeof(pwd) - 5; // "PWD="(4字节) + 结束符(1字节)
    if (strlen(temp) > max_path_len) {
        temp[max_path_len] = '\0'; // 截断过长路径
    }
    snprintf(pwd, sizeof(pwd), "PWD=%s", temp);
    putenv(pwd);

    // 提取最后一级目录名显示
    std::string pwd_lable = temp;
    const std::string pathsep = "/";
    auto pos = pwd_lable.rfind(pathsep);
    if (pos == std::string::npos) {
        return "none";
    }
    pwd_lable = pwd_lable.substr(pos + pathsep.size());
    return pwd_lable.empty() ? "/" : pwd_lable;
}

// 获取用户主目录（修复NULL初始化string问题）
std::string GetHomePath() {
    const char* home_env = getenv("HOME");
    if (home_env == NULL || *home_env == '\0') {
        return "/"; // 兜底返回根目录
    }
    return home_env;
}

// 打印命令提示符（[用户名@主机名 目录]# ）
void PrintCommandPrompt() {
    std::string user = GetUsername();
    std::string host = GetHostname();
    std::string current_dir = Getpwd();
    printf("[%s@%s %s]# ", user.c_str(), host.c_str(), current_dir.c_str());
}

// 读取用户输入的命令字符串
bool GetCommandString(char cmd_str_buff[], int len) {
    if (cmd_str_buff == NULL || len <= 0) {
        return false;
    }

    char* res = fgets(cmd_str_buff, len, stdin);
    if (res == NULL) { // 读取失败（如EOF）
        return false;
    }

    // 处理空输入（仅按回车）
    size_t cmd_len = strlen(cmd_str_buff);
    if (cmd_len <= 1) {
        cmd_str_buff[0] = '\0';
        return false;
    }

    cmd_str_buff[cmd_len - 1] = 0; // 去掉末尾换行符
    return true;
}

// 处理重定向逻辑（解析>、>>、<符号）
void CheckRedir(char cmd[]) {
    char* start = cmd;
    if (cmd == nullptr || *cmd == '\0') { // 空命令直接返回
        redir_type = NONE_REDIR;
        filename.clear();
        return;
    }
    char* end = cmd + strlen(cmd) - 1;

    // 重置重定向状态
    redir_type = NONE_REDIR;
    filename.clear();

    while (start <= end) {
        if (*start == '>') {
            if (*(start + 1) == '>') { // 追加重定向（>>）
                redir_type = APPEND_REDIR;
                *start = '\0'; // 截断命令，分离重定向部分
                start += 2;
                TrimSpace(start);
                // 检查文件名有效性
                if (start > end || *start == '\0') {
                    fprintf(stderr, "错误：>> 后缺少文件名\n");
                    redir_type = NONE_REDIR;
                    filename.clear();
                    break;
                }
                filename = start;
                break;
            } else { // 输出重定向（>）
                redir_type = OUTPUT_REDIR;
                *start = '\0';
                start++;
                TrimSpace(start);
                if (start > end || *start == '\0') {
                    fprintf(stderr, "错误：> 后缺少文件名\n");
                    redir_type = NONE_REDIR;
                    filename.clear();
                    break;
                }
                filename = start;
                break;
            }
        } else if (*start == '<') { // 输入重定向（<）
            redir_type = INPUT_REDIR;
            *start = '\0';
            start++;
            TrimSpace(start);
            if (start > end || *start == '\0') {
                fprintf(stderr, "错误：< 后缺少文件名\n");
                redir_type = NONE_REDIR;
                filename.clear();
                break;
            }
            filename = start;
            break;
        } else {
            start++;
        }
    }
}

// 初始化全局变量（每次解析命令前调用）
void InitGlobal() {
    gargc = 0;
    memset(gargv, 0, sizeof(gargv));
    filename.clear();
    redir_type = NONE_REDIR;
}

// 解析命令字符串为参数数组（修复参数截断问题）
bool ParseCommandString(char cmd[]) {
    if (cmd == NULL || *cmd == '\0') {
        return false;
    }

#define SEP " "
    // 解析第一个参数
    gargv[gargc] = strtok(cmd, SEP);
    if (gargv[0] == NULL) { // 第一个参数为空，解析失败
        gargc = 0;
        return false;
    }

    // 解析剩余参数（避免提前置空导致截断）
    while (true) {
        gargc++;
        gargv[gargc] = strtok(NULL, SEP);
        // 终止条件：参数为NULL 或 达到最大参数数量
        if (gargv[gargc] == NULL || gargc >= ARGS - 1) {
            break;
        }
    }
    // 确保参数数组以NULL结尾（execvp要求）
    gargv[gargc + 1] = NULL;

#ifdef DEBUG // 调试模式：打印参数解析结果
    printf("参数数量：%d\n", gargc);
    printf("--------------------\n");
    for (int i = 0; gargv[i]; i++) {
        printf("参数[%d]: %s\n", i, gargv[i]);
    }
    printf("--------------------\n");
#endif

    return true;
}

// 创建子进程执行外部命令（处理重定向）
void ForkAndExec() {
    pid_t id = fork();
    if (id < 0) { // fork失败
        perror("FORK失败");
        return;
    } else if (id == 0) { // 子进程
        int fd = -1;
        // 处理输出重定向
        if (redir_type == OUTPUT_REDIR) {
            fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if (fd < 0) {
                perror("输出文件打开失败");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO); // 重定向标准输出到文件
        }
        // 处理输入重定向
        else if (redir_type == INPUT_REDIR) {
            fd = open(filename.c_str(), O_RDONLY);
            if (fd < 0) {
                perror("打开输入文件失败");
                exit(1);
            }
            dup2(fd, STDIN_FILENO); // 重定向标准输入到文件
        }
        // 处理追加重定向
        else if (redir_type == APPEND_REDIR) {
            fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
            if (fd < 0) {
                perror("打开追加文件失败");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO); // 重定向标准输出到文件
        }

        // 执行外部命令（若返回则执行失败）
        execvp(gargv[0], gargv);
        // 错误处理：关闭文件描述符并退出
        perror("命令执行失败");
        if (fd != -1) {
            close(fd);
        }
        exit(1);
    } else { // 父进程：等待子进程结束
        int status = 0;
        pid_t rid = waitpid(id, &status, 0);
        if (rid > 0) {
            lastcode = WEXITSTATUS(status); // 保存子进程退出码
        }
    }
}

// 处理内建命令（cd、echo、exit）
bool BuiltInCommandExec() {
    if (gargc == 0 || gargv[0] == NULL || *gargv[0] == '\0') {
        return false;
    }

    std::string cmd = gargv[0];

    // 处理cd命令（切换目录）
    if (cmd == "cd") {
        std::string target_dir;
        if (gargc == 2) {
            // 检查参数有效性（避免NULL）
            if (gargv[1] == NULL || *gargv[1] == '\0') {
                fprintf(stderr, "cd: 无效的目录参数\n");
                lastcode = 1;
                return true;
            }
            target_dir = gargv[1];
            if (target_dir == "~") { // 处理~符号（主目录）
                target_dir = GetHomePath();
            }
        } else if (gargc == 1) { // 无参数：默认切换到主目录
            target_dir = GetHomePath();
        } else { // 参数过多
            fprintf(stderr, "cd: 参数过多\n");
            lastcode = 1;
            return true;
        }

        // 执行目录切换
        if (chdir(target_dir.c_str()) != 0) {
            perror("cd失败");
            lastcode = 1;
        } else {
            lastcode = 0;
        }
        return true; // 标记为内建命令
    }

    // 处理echo命令（输出内容）
    else if (cmd == "echo") {
        if (gargc >= 2) {
            if (gargv[1] == NULL || *gargv[1] == '\0') {
                printf("\n");
                lastcode = 0;
                return true;
            }
            std::string args = gargv[1];

            // 处理环境变量（$XXX）和退出码（$?）
            if (args[0] == '$') {
                if (args == "$?") { // 输出上一条命令退出码
                    printf("%d\n", lastcode);
                    lastcode = 0;
                } else { // 输出环境变量
                    const char* env_name = args.c_str() + 1;
                    const char* env_val = getenv(env_name);
                    if (env_val) {
                        printf("%s\n", env_val);
                    } else {
                        printf("\n");
                    }
                    lastcode = 0;
                }
            } else { // 输出普通字符串（参数间加空格）
                for (int i = 1; i < gargc; i++) {
                    if (gargv[i] != NULL) {
                        printf("%s ", gargv[i]);
                    }
                }
                printf("\n");
                lastcode = 0;
            }
        } else { // 无参数：输出空行
            printf("\n");
            lastcode = 0;
        }
        return true; // 标记为内建命令
    }

    // 处理exit命令（退出shell）
    else if (cmd == "exit") {
        printf("退出shell\n");
        exit(0);
    }

    // 非内建命令：返回false，后续执行ForkAndExec
    return false;
}