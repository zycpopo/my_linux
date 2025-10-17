#include<iostream>
#include<cstring>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<stdlib.h>

const key_t SHM_KEY=0x1234;
const int SHM_SIZE=1024;

int main()
{
  int shmid;
  char* shm_addr;

  shmid=shmget(SHM_KEY,SHM_SIZE,0666);
  if(shmid==-1)
  {
    std::cerr<<"获取共享内存失败，请先运行A程序"<<std::endl;
    exit(EXIT_FAILURE);
  }

  shm_addr = static_cast<char*>(shmat(shmid, nullptr, 0));
  if (shm_addr == reinterpret_cast<char*>(-1)) 
  {
      std::cerr << "映射共享内存失败！" << std::endl;
      exit(EXIT_FAILURE);
  }

  std::cout<<"进程B:从共享内存中读取数据"<<shm_addr<<std::endl;

  if(shmdt(shm_addr)==-1)
  {
      std::cerr<<"解除共享内存失败!"<<std::endl;
      exit(EXIT_FAILURE);
  }

  if (shmctl(shmid, IPC_RMID, nullptr) == -1) 
  {
      std::cerr << "删除共享内存失败！" << std::endl;
      exit(EXIT_FAILURE);
  }
}