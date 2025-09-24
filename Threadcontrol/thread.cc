#include "thread.hpp"  // 包含自定义线程类头文件
#include <iostream>
#include <chrono>      // 用于简单延时（替代sleep，精度更高）

// 示例1：普通函数（作为线程任务）
void task1()
{
    for (int i = 0; i < 3; ++i)
    {
        std::cout << "[Task1] running | count: " << i+1 
                  << " | kernel lwpid: " << get_lwp_id() << std::endl;
        // 延时500毫秒（模拟任务耗时）
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

// 示例2：lambda表达式（作为线程任务，更灵活）
void test_lambda_thread()
{
    std::string msg = "Hello from Lambda Thread";
    // 创建线程对象，任务为lambda表达式
    Thread lambda_thread([&msg]() {
        for (int i = 0; i < 2; ++i)
        {
            std::cout << "[LambdaTask] " << msg << " | count: " << i+1 << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        }
    }, "Lambda-Thread");

    lambda_thread.Start();  // 启动线程
    lambda_thread.Join();   // 等待线程结束
}

int main()
{
    // 1. 使用普通函数作为任务创建线程
    Thread t1(task1, "Task1-Thread");
    t1.Start();  // 启动线程t1
    t1.Join();   // 等待t1结束

    std::cout << "------------------------" << std::endl;

    // 2. 使用lambda表达式作为任务创建线程
    test_lambda_thread();

    return 0;
}