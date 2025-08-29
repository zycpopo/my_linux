#ifndef PROCESS_POOL_HPP
#define PROCESS_POOL_HPP

#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <functional>
#include <stdexcept>
#include <iostream>
#include "Task.hpp"

class channel
{
public:
  int _wfd;
  pid_t _sub_target;
  bool _in_use;

  channel()
      : _wfd(-1), _sub_target(-1), _in_use(false) {}

  void close()
  {
    if (_wfd != -1)
    {
      ::close(_wfd);
      _wfd = -1;
    }
  }

  void wait()
  {
    if (_sub_target != -1)
    {
      waitpid(_sub_target, nullptr, 0);
      _sub_target = -1;
    }
  }
};

class processpool
{
private:
  int _subproc_num;
  std::vector<channel> _channels;
  bool _is_running;

public:
  // 构造函数
  processpool(int num = 5)
      : _subproc_num(num),
        _is_running(false)
  {
    if (num <= 0)
    {
      throw std::invalid_argument("子进程数量必须大于0");
    }
  }
  ~processpool()
  {
    if (_is_running)
    {
      waitsubprocesses();
    }
  }

  // 初始化进程池
  void initprocesspool(std::function<void(int)> subproc_callback)
  {
    if (_is_running)
    {
      return;
    }

    _channels.resize(_subproc_num);

    for (int i = 0; i < _subproc_num; ++i)
    {
      int pipefd[2];
      // 创建管道
      if (pipe(pipefd) == -1)
      {
        throw std::runtime_error("创建管道失败");
      }

      // 创建子进程
      pid_t pid = fork();
      if (pid == -1)
      {
        ::close(pipefd[0]);
        ::close(pipefd[1]);
        throw std::runtime_error("创建子进程失败");
      }

      if (pid == 0)
      { // 子进程
        // 关闭所有继承的写端
        for (int j = 0; j < i; ++j)
        {
          ::close(_channels[j]._wfd);
        }

        // 关闭当前管道的写端
        ::close(pipefd[1]);

        // 执行子进程回调函数
        subproc_callback(pipefd[0]);

        // 子进程完成任务后退出
        ::close(pipefd[0]);
        exit(0);
      }
      else
      { // 父进程
        // 关闭管道的读端
        ::close(pipefd[0]);

        // 保存写端和子进程ID
        _channels[i]._wfd = pipefd[1];
        _channels[i]._sub_target = pid;
        _channels[i]._in_use = false;
      }
    }
    _is_running = true;
  }

  void pollingctrlsubprocess(int task_count)
  {
    if (!_is_running)
    {
      throw std::runtime_error("进程池未初始化");
    }

    int current = 0;
    for (int i = 0; i < task_count; ++i)
    {
      // 轮询选择子进程
      int channel_idx = current % _subproc_num;
      current++;

      // 生成随机任务编号(0-3)
      int task_code = rand() % 4;

      // 向子进程发送任务编号
      if (write(_channels[channel_idx]._wfd, &task_code, sizeof(task_code)) != sizeof(task_code))
      {
        std::cerr << "发送任务失败" << std::endl;
      }
      else
      {
        std::cout << "父进程[" << getpid() << "] 向子进程["
                  << _channels[channel_idx]._sub_target << "] 分配任务 " << task_code << std::endl;
      }

      // 简单休眠，模拟任务分配间隔
      usleep(100000);
    }
  }

  void waitsubprocesses()
  {
    if (!_is_running)
    {
      return;
    }

    // 关闭所有写端，通知子进程可以退出
    for (auto &channel : _channels)
    {
      channel.close();
    }

    // 等待所有子进程退出
    for (auto &channel : _channels)
    {
      channel.wait();
    }

    _channels.clear();
    _is_running = false;
    std::cout << "所有子进程已退出" << std::endl;
  }
};

#endif