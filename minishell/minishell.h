#ifndef MINISHELL_H
#define MINISHELL_H

#include <string>

// 重定向类型枚举（避免魔法数字）
#define NONE_REDIR  0   // 无重定向
#define OUTPUT_REDIR 1  // 输出重定向（>）
#define APPEND_REDIR 2  // 追加重定向（>>）
#define INPUT_REDIR  3  // 输入重定向（<）

// 命令参数最大数量（与cpp文件一致）
#define ARGS 100

// 全局变量声明（extern表示仅声明，定义在cpp中）
extern char* gargv[ARGS];       // 命令参数数组
extern int gargc;               // 命令参数个数
extern char pwd[1024];          // 存储PWD环境变量
extern int lastcode;            // 上一条命令的退出码
extern std::string filename;    // 重定向文件名
extern int redir_type;          // 当前重定向类型

// 函数声明（与cpp文件实现完全对应）
void Debug();
std::string GetUsername();
std::string GetHostname();
std::string Getpwd();           // 统一小写p，与cpp实现一致
std::string GetHomePath();
void PrintCommandPrompt();
bool GetCommandString(char cmd_str_buff[], int len);
void CheckRedir(char cmd[]);
void InitGlobal();
bool ParseCommandString(char cmd[]);
void ForkAndExec();
bool BuiltInCommandExec();

// 声明TrimSpace宏（供cpp文件使用）
#define TrimSpace(start) \
    do { \
        if (start != NULL) { \
            while (isspace((unsigned char)*start)) { \
                start++; \
            } \
        } \
    } while (0)

#endif // MINISHELL_H