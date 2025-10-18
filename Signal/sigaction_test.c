#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// 自定义信号处理函数，打印触发的信号值
void sigcb(int signo) {
    printf("捕获到信号值: %d (SIGINT)\n", signo);
}

int main() {
    struct sigaction act, oldact;  // 定义信号动作结构体

    // 初始化信号处理结构体
    act.sa_handler = sigcb;        // 设置自定义处理函数
    sigemptyset(&act.sa_mask);     // 清空信号掩码（处理期间不阻塞其他信号）
    act.sa_flags = 0;              // 无特殊标志（默认行为）

    // 注册SIGINT信号的处理方式
    // 成功返回0，失败返回-1
    if (sigaction(SIGINT, &act, &oldact) == -1) {
        perror("sigaction failed");
        return 1;
    }

    printf("程序运行中，按Ctrl+C触发SIGINT信号...\n");
    printf("按Ctrl+\\退出程序（触发SIGQUIT）\n");

    // 死循环等待信号触发
    while (1) {
        sleep(1);  // 休眠1秒，降低CPU占用
    }

    return 0;
}