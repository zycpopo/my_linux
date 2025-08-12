#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid = fork();  // 创建子进程

    if (pid < 0) {
        // 创建子进程失败
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // 子进程逻辑
        printf("我是子进程，PID: %d，父进程PID: %d\n", getpid(), getppid());
        
        // 子进程休眠10秒，确保父进程先退出
        printf("子进程将休眠10秒，等待父进程退出...\n");
        sleep(10);
        
        // 再次打印父进程ID，此时应变为1号进程
        printf("子进程休眠结束，当前父进程PID: %d\n", getppid());
        exit(0);
    } 
    else {
        // 父进程逻辑
        printf("我是父进程，PID: %d，子进程PID: %d\n", getpid(), pid);
        
        // 父进程立即退出，使子进程成为孤儿进程
        printf("父进程即将退出...\n");
        exit(0);
    }
}
    
