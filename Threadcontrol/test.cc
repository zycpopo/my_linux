#include<iostream>
#include<string>
#include<pthread.h>
#include<unistd.h>

void* start_routine(void* args)
{
  std::string name=static_cast<const char*>(args);
  while(true)
  {
    std::cout<<"我是一个新线程，"<<pthread_self()<<"threadname is"<<name<<std::endl;
    sleep(1);
    break;
  }

  return (void*)11;
}

int main()
{
  pthread_t tid;
  pthread_create(&tid,nullptr,start_routine,(void*)"thread-1");

  void *ret=nullptr;
  //获取线程结束的返回值
  pthread_join(tid,&ret);
  std::cout<<"join success:"<<(long long int)(ret)<<std::endl;
  return 0;
}