# 定义编译器
CC = gcc
# 编译选项
CFLAGS = -Wall -g

# 可执行文件目标
TARGETS = write read

# 默认目标，编译所有可执行文件
all: $(TARGETS)

# 编译 write 程序
write: write.c
	$(CC) $(CFLAGS) -o $@ $^

# 编译 read 程序
read: read.c
	$(CC) $(CFLAGS) -o $@ $^

# 清理生成的可执行文件和可能的临时文件
clean:
	rm -f $(TARGETS) abc.bak tp