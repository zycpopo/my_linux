#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();  // 创建子进程

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // 子进程：执行程序替换，变为ls -l -a
        printf("子进程(PID: %d)即将替换为ls程序...\n", getpid());
        
        // execlp函数：从PATH环境变量查找程序，参数列表以NULL结束
        // 第一个参数：程序名(ls)
        // 后续参数：命令行参数(-l, -a)，最后必须是NULL
        execlp("ls", "ls", "-l", "-a", NULL);
        
        // 如果execlp执行成功，下面的代码不会执行
        perror("execlp failed");  // 只有替换失败才会执行
        exit(1);
    } 
    else {
        // 父进程：等待子进程执行完毕
        printf("父进程(PID: %d)等待子进程(PID: %d)执行...\n", getpid(), pid);
        wait(NULL);  // 等待子进程结束
        printf("父进程：子进程执行完毕\n");
    }

    return 0;
}

