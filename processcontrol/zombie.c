#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();  // 创建子进程

    if (pid < 0) {
        // 创建子进程失败
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // 子进程逻辑：立即退出，成为僵尸进程
        printf("子进程 (PID: %d) 启动，即将退出...\n", getpid());
        exit(0);  // 子进程终止
    } 
    else {
        // 父进程逻辑：不调用wait()回收子进程，进入无限循环
        printf("父进程 (PID: %d) 启动，子进程PID: %d\n", getpid(), pid);
        printf("父进程不回收子进程，子进程将成为僵尸进程\n");
        printf("请在另一个终端用ps命令查看僵尸进程（状态为Z）\n");
        
        // 父进程无限循环，保持运行（不退出也不回收子进程）
        while (1) {
            sleep(3);  // 每3秒休眠一次，避免占用过多CPU
        }
    }
}

