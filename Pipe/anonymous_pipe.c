#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];  // 管道文件描述符数组，pipefd[0]为读端，pipefd[1]为写端
    pid_t pid;
    char buffer[1024];  // 用于接收数据的缓冲区

    // 创建匿名管道
    if (pipe(pipefd) == -1) {
        perror("pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // 创建子进程
    pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // 子进程
        close(pipefd[1]);  // 子进程不需要写端，关闭写端

        // 从管道读取数据
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (bytes_read == -1) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }

        buffer[bytes_read] = '\0';  // 添加字符串结束符
        printf("子进程收到消息: %s\n", buffer);

        close(pipefd[0]);  // 读取完成后关闭读端
        exit(EXIT_SUCCESS);
    } else {  // 父进程
        close(pipefd[0]);  // 父进程不需要读端，关闭读端

        const char *message = "你好，这是来自父进程的消息！";
        // 向管道写入数据
        if (write(pipefd[1], message, strlen(message)) == -1) {
            perror("write failed");
            exit(EXIT_FAILURE);
        }
        printf("父进程已发送消息\n");

        close(pipefd[1]);  // 写入完成后关闭写端
        wait(NULL);        // 等待子进程结束
        exit(EXIT_SUCCESS);
    }
}

