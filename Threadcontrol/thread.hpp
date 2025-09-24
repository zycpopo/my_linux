#ifndef __THREAD_HPP__  // 防止头文件重复包含（头文件保护宏）
#define __THREAD_HPP__

// 引入依赖头文件
#include <iostream>         // 用于输入输出
#include <string>           // 用于字符串处理
#include <pthread.h>        // POSIX线程库核心头文件（提供线程创建/回收等接口）
#include <unistd.h>         // 提供系统调用基础支持
#include <functional>       // 用于std::function，实现灵活的函数对象封装
#include <sys/syscall.h>    // 用于SYS_gettid宏定义（获取内核态线程ID）

// 宏定义：获取当前线程的内核态ID（LWP ID）
// 说明：pthread_self()返回用户态线程ID，而SYS_gettid系统调用返回内核中轻量级进程（线程）的真实ID
#define get_lwp_id() syscall(SYS_gettid)

// 类型别名：定义线程执行函数的类型
// 说明：func_t是一个无参数、无返回值的函数对象类型，支持lambda、函数指针、绑定函数等
using func_t = std::function<void()>;

// 全局常量：默认线程名称（当用户未指定线程名时使用）
const std::string threadnamedefault = "None-Name";

// 线程类封装：基于POSIX线程库，提供面向对象的线程操作接口
class Thread
{
public:
    // 构造函数：初始化线程对象（关联任务函数和线程名）
    // 参数1：func - 线程要执行的任务函数（func_t类型，支持多种函数形式）
    // 参数2：name - 线程名称（默认使用threadnamedefault）
    Thread(func_t func, const std::string &name = threadnamedefault)
        : _name(name),          // 初始化线程名称
          _func(func),          // 绑定线程要执行的任务函数
          _isrunning(false)     // 初始化线程运行状态：false表示未启动
    {
        // 构造函数仅创建线程对象，不启动线程（启动逻辑在Start()中）
        std::cout << "create thread obj success | thread name: " << _name << std::endl;
    }

    // 静态成员函数：线程入口函数（必须是静态的，因为pthread_create要求全局/静态函数）
    // 参数：args - 传递给线程的参数（此处为当前Thread对象的this指针）
    // 返回值：void* - 线程退出时的返回值（此处未使用，返回NULL）
    static void *start_routine(void *args)
    {
        // 步骤1：将void*类型的参数转换为Thread*，获取当前线程对象的指针
        // 说明：因为静态函数无法直接访问非静态成员，通过this指针间接访问
        Thread *self = static_cast<Thread *>(args);

        // 步骤2：更新线程状态为“运行中”
        self->_isrunning = true;

        // 步骤3：获取当前线程的内核态ID（LWP ID），存入对象成员
        self->_lwpid = get_lwp_id();

        // 步骤4：执行用户绑定的任务函数（核心逻辑）
        // 说明：_func是用户通过构造函数传入的任务，此处真正执行线程工作
        self->_func();

        // 步骤5：线程任务执行完毕，主动退出（避免内存泄漏）
        // 说明：pthread_exit()会终止当前线程，返回值为(void*)0（此处未使用）
        pthread_exit((void *)0);
    }

    // 成员函数：启动线程（创建并启动POSIX线程）
    void Start()
    {
        // 调用pthread_create创建线程
        // 参数1：&_tid - 输出参数，存储创建后的用户态线程ID（pthread_t类型）
        // 参数2：nullptr - 线程属性，使用默认属性（如栈大小、调度策略等）
        // 参数3：start_routine - 线程入口函数（静态成员函数）
        // 参数4：this - 传递给入口函数的参数（当前Thread对象指针）
        int n = pthread_create(&_tid, nullptr, start_routine, this);

        // 检查线程创建结果：n==0表示创建成功
        if (n == 0)
        {
            std::cout << "run thread success | thread name: " << _name 
                      << " | user态tid: " << _tid << std::endl;
        }
        else
        {
            // 线程创建失败（如资源不足），可根据需要添加错误处理
            std::cerr << "run thread failed | thread name: " << _name << std::endl;
        }
    }

    // （注释掉的功能）成员函数：强制终止线程（慎用！可能导致资源泄漏）
    // 说明：pthread_cancel会强制终止线程，但如果线程持有锁或未释放资源，会引发问题
    // void Die()
    // {
    //     pthread_cancel(_tid);
    // }

    // 成员函数：等待线程结束并回收资源（阻塞调用）
    // 说明：调用后主线程会阻塞，直到当前线程执行完毕，避免僵尸线程
    void Join()
    {
        // 先判断线程状态：如果线程未启动（_isrunning=false），直接返回（无需等待）
        if (!_isrunning)
        {
            std::cout << "join skipped: thread not running | thread name: " << _name << std::endl;
            return;
        }

        // 调用pthread_join等待线程结束
        // 参数1：_tid - 要等待的线程的用户态ID
        // 参数2：nullptr - 线程退出返回值（此处不关心，设为NULL）
        int n = pthread_join(_tid, nullptr);

        // 检查等待结果：n==0表示等待成功（线程正常结束并回收）
        if (n == 0)
        {
            std::cout << "pthread_join success | thread name: " << _name 
                      << " | kernel态lwpid: " << _lwpid << std::endl;
            // 线程回收后，更新状态为“未运行”
            _isrunning = false;
        }
        else
        {
            std::cerr << "pthread_join failed | thread name: " << _name << std::endl;
        }
    }

    // 析构函数：线程对象销毁时调用
    // 说明：析构函数不主动回收线程（避免强制终止），线程回收需显式调用Join()
    ~Thread()
    {
        // 此处无需复杂操作，因为线程资源已通过Join()回收
        // 若未调用Join()，线程可能成为僵尸线程（需用户确保调用Join()）
        std::cout << "destroy thread obj | thread name: " << _name << std::endl;
    }

private:
    bool _isrunning;   // 线程运行状态：true=运行中，false=未启动/已结束（避免重复Join）
    pthread_t _tid;    // 用户态线程ID（由pthread_create生成，仅在用户态有效）
    pid_t _lwpid;      // 内核态线程ID（LWP ID，由SYS_gettid获取，对应内核轻量级进程）
    std::string _name; // 线程名称（用于日志输出，便于调试）
    func_t _func;      // 线程要执行的任务函数（用户通过构造函数绑定）
};

#endif  // 头文件保护宏结束