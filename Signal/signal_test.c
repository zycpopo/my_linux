#include <stdio.h>
#include <signal.h>

// 自定义信号处理函数sigcb，打印触发的信号值
void sigcb(int signo) {
    printf("触发的信号值为：%d\n", signo);
}

int main() {
    // 注册SIGINT信号（Ctrl+C触发）的处理函数为sigcb
    signal(SIGINT, sigcb);

    printf("程序正在运行，按Ctrl+C触发SIGINT信号...\n");
    printf("按Ctrl+\\可退出程序（触发SIGQUIT信号）\n");

    // 死循环等待信号触发
    while (1);

    return 0;
}