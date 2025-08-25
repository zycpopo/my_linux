#include<iostream>
#include<string.h>
#include<ctype.h>
#include<cstdio>
#include<cstring>
#include<stdlib.h>
#include<unistd.h>
#include<string>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/wait.h>

//定义命令行参数最大数量
#define ARGS 100

//定义全局变量
int lastcode=0;
char pwd[1024];
int gargc=0;
char* gargv[ARGS]={NULL};


static std::string GetUsername()
{
  std::string username=getenv("USER");
  return username.empty()?"none":username;
}

static std::string GetHostname()
{
  std::string hostname=getenv("HOSTNAME");
  return hostname.empty()?"none":hostname;
}

//获取当前工作目录
static std::string GetPwd()
{
  char temp[1024];

  if(getcwd(temp,sizeof(temp))==NULL)
  {
    perror("getcwd error");
    return "Unknown";
  }

  snprintf(pwd,sizeof(pwd),"PWD=%s",temp);
  putenv(pwd);

  std::string pwd_lable=temp;
  const std::string pathsep="/";
  auto pos =pwd_lable.rfind(pathsep);

  if(pos==std::string::npos)
  {
    return "none";
  }
  //只显示最后一级目录
  pwd_lable=pwd_lable.substr(pos+pathsep.size());
  return pwd_lable.empty()?"/":pwd_lable;//这里如果是没有就默认是“/”根目录
}

//获取用户主目录路径
static std::string GetHomePath()
{
  std::string home=getenv("HOME");
  return home.empty()?"/":home;
}

void PrintCommandprompt()
{
  std::string user=GetUsername();
  std::string host=GetHostname();
  std::string current_dir=GetPwd();

  printf("[%s@%s %s]# ",user.c_str(),host.c_str(),current_dir.c_str());
}

bool GetCommandString(char cmd_str_buff[],int len)
{
  if(cmd_str_buff==NULL || len<=0)
  {
    return false;
  }

  char *res=fgets(cmd_str_buff,len,stdin);
  if(res==NULL)
  {
    return false;
  }

  cmd_str_buff[strlen(cmd_str_buff)-1]=0;//将末尾的换行符去掉

  return strlen(cmd_str_buff)==0?false:true;
}

//宏定义
#define TrimSpace(start)\
            do{\
              while(isspace(*start))\
              {\
                start++;\
              }\
            }while(0)
//去掉字符串开头的空格

