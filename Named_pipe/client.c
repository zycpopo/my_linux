#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define PIPE_NAME "/tmp/my_named_pipe"
#define BUF_SIZE 1024

int main() 
{
    // 1. 打开管道（写模式）
    int pipe_fd = open(PIPE_NAME, O_WRONLY);
    if (pipe_fd == -1) {
        perror("open pipe failed");
        exit(1);
    }

    // 2. 写入数据
    const char* msg = "Hello, Named Pipe!";
    if (write(pipe_fd, msg, strlen(msg)) == -1) {
        perror("write failed");
        close(pipe_fd);
        exit(1);
    }
    printf("Message sent: %s\n", msg);

    // 3. 关闭管道
    close(pipe_fd);
    return 0;
}