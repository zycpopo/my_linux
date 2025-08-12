#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
    pid_t pid = fork();  // 创建子进程

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // 子进程逻辑
        printf("i am child process (PID: %d)\n", getpid());
        sleep(5);  // 等待5秒
        exit(10);  // 子进程退出，退出码为10
    } 
    else {
        // 父进程逻辑
        printf("i am parent process (PID: %d), waiting for child (PID: %d)...\n", getpid(), pid);
        
        int status;
        // 等待指定子进程(pid)退出，阻塞式等待
        pid_t wait_ret = waitpid(pid, &status, 0);

        if (wait_ret == -1) {
            perror("waitpid failed");
            exit(1);
        }

        // 解析子进程退出状态
        printf("\nParent process detected child exit:\n");
        
        // 检查是否正常退出
        if (WIFEXITED(status)) {
            printf("Child exited normally\n");
            printf("Exit code: %d\n", WEXITSTATUS(status));
        }
        // 检查是否被信号终止
        else if (WIFSIGNALED(status)) {
            printf("Child killed by signal\n");
            printf("Signal number: %d\n", WTERMSIG(status));
            // 检查是否产生core dump
            if (WCOREDUMP(status)) {
                printf("Core dump generated\n");
            }
        }
    }

    return 0;
}

