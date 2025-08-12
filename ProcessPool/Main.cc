#include "ProcessPool.hpp"
#include "Task.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>

// 子进程回调函数：循环等待并执行任务
void SubProcessMain(int rfd) {
    std::cout << "子进程[" << getpid() << "] 启动，等待任务..." << std::endl;
    
    int task_code;
    // 循环读取任务编号，直到管道关闭
    while (read(rfd, &task_code, sizeof(task_code)) == sizeof(task_code)) {
        // 检查任务编号是否有效
        if (task_code >= 0 && task_code < g_tasks.tasks.size()) {
            // 执行对应的任务
            g_tasks.tasks[task_code]();
        } else {
            std::cerr << "子进程[" << getpid() << "] 收到无效任务编号: " << task_code << std::endl;
        }
    }
    
    std::cout << "子进程[" << getpid() << "] 收到退出信号，即将退出" << std::endl;
    ::close(rfd);
}

int main() {
    std::cout << "主进程[" << getpid() << "] 启动" << std::endl;
    
    try {
        // 设置随机数种子
        srand(time(nullptr));
        
        // 创建进程池，包含5个子进程
        processpool pool(5);
        
        // 初始化进程池，传入子进程回调函数
        pool.initprocesspool(SubProcessMain);
        
        // 向子进程分配10个任务
        pool.pollingctrlsubprocess(10);
        
        // 等待所有子进程完成任务并退出
        pool.waitsubprocesses();
    } catch (const std::exception& e) {
        std::cerr << "发生错误: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "主进程[" << getpid() << "] 退出" << std::endl;
    return 0;
}
