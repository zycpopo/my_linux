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

#define NONE_REDIR 0  //无重定向
#define OUTPUT_REDIR 1//输出重定向(>)
#define APPEND_REDIR 2//追加重定向(>>)
#define INPUT_REDIR 3 //输入重定向(<)

std::string filename;
int redir_type=NONE_REDIR;//默认无重定向

void Debug()
{
    printf("hello shell!\n");
}

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

void CheckRedir(char cmd[])
{
  char *start=cmd;
  char *end=cmd+strlen(cmd)-1;

  while(start<=end)
  {
    if(*start=='>')
    {
      if(*(start+1)=='>')//处理追加重定向
      {
          redir_type=APPEND_REDIR;
          *start='\0';
          start+=2;
          TrimSpace(start);
          filename= start;
          break;
      }
      else//输出重定向
      {
          redir_type=OUTPUT_REDIR;
          *start='\0';
          start++;
          TrimSpace(start);
          filename=filename;
          break;
      }
    }
    else if(*start=='<')//输入重定向
    {
      redir_type=INPUT_REDIR;
      *start='\0';
      start++;
      TrimSpace(start);
      filename=start;
      break;
    }
    else
    {
      start++;
    }
  }
}

//初始化全局变量
void InitGloal()
{
  gargc=0;
  memset(gargv,0,sizeof(gargv));
  filename.clear();
  redir_type=NONE_REDIR;
}

//解析命令字符串为参数数组
bool ParseCommandString(char cmd[])
{
  if(cmd==NULL) return false;
  
#define SEP " "
    gargv[gargc]=strtok(cmd,SEP);

    //继续解析剩余参数
    while((gargv[gargc]==strtok(NULL,SEP))!=NULL)
    {
      gargc++;
      if(gargc>=ARGS-1)
      {
        break;
      }

      gargv[gargc]=NULL;
    }
#ifdef DEBUG
    printf("参数数量：%d\n",gargc);
    printf("--------------------\n");
    for(int i=0;gargv[i];i++)
    {
      printf("参数[%d]: %s\n",i,gargv[i]);
    }
    printf("--------------------\n");
#endif

    return true;
}

//创建子进程并执行命令
void ForkAndExec()
{
  pid_t id=fork();
  if(id<0)
  {
    perror("FORK失败");
    return;
  }
  else if(id==0)
  {
    int fd=-1;
    if(redir_type==OUTPUT_REDIR)
    {
      fd=open(filename.c_str(),O_CREAT | O_WRONLY | O_TRUNC,0666);
      if(fd<0){
        perror("输出文件打开失败");
        exit(1);
      }
      dup2(fd, STDOUT_FILENO);  // 将标准输出重定向到文件
    }
    else if (redir_type == INPUT_REDIR)
    {
            // 输入重定向：只读方式打开文件
            fd = open(filename.c_str(), O_RDONLY);
            if (fd < 0) {
                perror("打开输入文件失败");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);   // 将标准输入重定向到文件
    }
    else if (redir_type == APPEND_REDIR)
    {
            // 追加重定向：创建或追加到文件
            fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
            if (fd < 0) {
                perror("打开追加文件失败");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);  // 将标准输出重定向到文件
    }
        execvp(gargv[0], gargv);
        
        // 如果execvp返回，说明执行失败
        perror("命令执行失败");
        if (fd != -1) close(fd);  // 关闭文件描述符
        exit(1);
    }
    else
    {
        // 父进程：等待子进程结束
        int status = 0;
        pid_t rid = waitpid(id, &status, 0);
        if (rid > 0)
        {
            // 保存子进程的退出码
            lastcode = WEXITSTATUS(status);
        }
    }
}

//处理内建命令
bool BuiltInCommandExec()
{
  if(gargc==0 || gargv[0]==NULL)
    return false;

  std::string cmd=gargv[0];

  if(cmd == "cd")
  {
    std::string target_dir;

    if(gargc == 2)
    {
      target_dir = gargv[1];
      if(target_dir=="~")
      {
        target_dir=GetHomePath();
      }
    }
    else if(gargc==1)//默认切换到主目录
    {
      target_dir=GetHomePath();
    }
    else{
      fprintf(stderr,"cd: 参数过多\n");
      lastcode=1;
      return true;
    }

    //切换目录
    if(chdir(target_dir.c_str())!=0)
    {
      perror("cd失败");
      lastcode=1;
    }
    else
    {
      lastcode=0;
    }
  }

  else if(cmd=="echo")
  {
    if(gargc>=2)
    {
      std::string args=gargv[1];

      //处理环境变量和$?
      if(args[0] == '$')
      {
        if(args=="$?")
        {
          printf("%d\n",lastcode);
          lastcode=0;
        }
        else{
          //输出环境变量
          const char *env_val=getenv(&args[1]);
          if(env_val)
            printf("%s\n",env_val);
          else  printf("\n");

          lastcode=0;
        }
      }
      else 
      {
        for(int i=1;i<gargc;i++)
        {
          printf("%s",gargv[i]);
        }
        printf("\n");
        lastcode=0;
      }
    }
    else
    {
      //不带参数的echo输出空行
      printf("\n");
      lastcode=0;
    }
    return true;
  }

  //处理exit命令
  else if(cmd=="exit")
  {
    printf("退出shell\n");
    exit(0);
  }

  return false;
}