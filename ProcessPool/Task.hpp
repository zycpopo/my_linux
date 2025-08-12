#ifndef TASK_HPP
#define TASK_HPP

#include<functional>
#include<vector>
#include<iostream>

//定义任务类型
using task_t=std::function<void()>;

void download(){
  std::cout<<"子进程["<<getpid()<<"]执行下载任务.."<<std::endl;
}

void mysql(){
  std::cout << "子进程[" << getpid() << "] 执行数据库任务.." << std::endl;
}

void sync(){
  std::cout << "子进程[" << getpid() << "] 执行同步任务.." << std::endl;
}

void log(){
  std::cout << "子进程[" << getpid() << "] 执行日志任务.." << std::endl;
}

class taskinit
{
public:
  taskinit(){
      tasks.emplace_back(download);
      tasks.emplace_back(mysql);
      tasks.emplace_back(sync);
      tasks.emplace_back(log);
  }

  std::vector<task_t> tasks;
};

static taskinit g_tasks;

#endif 