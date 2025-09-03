#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PIPE_NAME "/tmp/my_named_pipe"
#define BUF_SIZE 1024

int main() 
{
    // 1. 创建命名管道（FIFO），权限：仅所有者可读写
    if (mkfifo(PIPE_NAME, 0600) == -1) {
        perror("mkfifo failed");
        exit(1);
    }
    printf("Pipe created: %s\n", PIPE_NAME);

    // 2. 打开管道（读模式）
    int pipe_fd = open(PIPE_NAME, O_RDONLY);
    if (pipe_fd == -1) {
        perror("open pipe failed");
        unlink(PIPE_NAME); // 失败时删除管道
        exit(1);
    }

    // 3. 读取数据
    char buf[BUF_SIZE];
    ssize_t bytes_read = read(pipe_fd, buf, BUF_SIZE);
    if (bytes_read > 0) {
        printf("Received: %.*s\n", (int)bytes_read, buf);
    }

    // 4. 关闭并删除管道
    close(pipe_fd);
    unlink(PIPE_NAME);
    printf("Pipe closed and deleted\n");
    return 0;
}