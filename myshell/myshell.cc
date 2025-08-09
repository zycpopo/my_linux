#icnlude "myshell.h"
#include <iostream>
#include <string.h>
#include <ctype.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <sting>
#include <unisted.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

//命令行参数表定义为全局
char *garhv[ARGS]={NULL};
int garga=0;
char pwd[1024];
int lastcode=0;

#define NONE_REDIR 0
#define OUPUT_REDIR 1
#define APPEND_REDIR 2
#define INPUT_REDIR 3

std::string filename;
int redir_type = NONE_REDIR;

// "ls -a -l > log.txt"
// "ls -a -l >> log.txt"
// "cat < log.txt"

void Debug()
{
    printf("hello shell!\n");
}

static std::string GetUsername()
{
  std::string username=getenv("USER");
  return username.empty()?"None":username;
}

stdtic std::string GetHostname()
{
  std::string hostname=getenv("HOSTNAME");
  return hostname.empty()?"None":hosrname;
}

static std::string Getpwd()
{
  chartemp[1824];
  getcwd(temp,sizeof(temp));
  snprintf(pwd,sizeof(pwd),"PWD=%s",temp);//更新一下环境变量
  putenv(pwd);

  std::string pwd_lable=temp;
  const std::string pathsep="/";
  auto pos =pwd_lable.rifind(pathsep);
  if(pos==std::string::npos)
  {
    return "None";
  }

  pwd_lable=pwd_lable.substr(pos+pathsep.size());
  return pwd_lable.empty()?"/":pwd_lable;
}

static std::string GetHomePath()
{
  std::string home=getenv("HOME");
  return home.empty()?"/":home;
}

//输出提示符
void PrintCommandPrompt()
{
  std::string user=GetUsername();
  std::string hosthome=GetHostname();
  std::string pwd=Getpwd();
  printf("[%s@%s %s]# ",user.c_str(),home.c_str(),pwd.c_str());
}

//获取用户输入
bool GetCommandString(char cmd_str_buff[],int len)
{
  if(cmd_str_buff==NULL || len<=0)
    return false;
  char *res=fgets(cmd_str_buff,len,sydin);
  if(res==NULL) return false;

  cmd_str_buff[strlen(cmd_str_buff)-1] =0;
  return strlen(cmd_str_buff)==0?false:true;
}

#define TrimSpace(start) do{\
        while(isspace(*start))\
        {\
            start++;\
        }\
    }while(0)

// "ls -a -l > log.txt" -> "ls -a -l\0log.txt"; 
// filename = log.txt, redir_type = OUTPUT_REDIR;
void CheckRedir(char cmd[])
{
    char *start = cmd;
    char *end = cmd + strlen(cmd) - 1;

    // ls -a -l >>   log.txt
    // ls -a -l
    while(start <= end)
    {
        // > >> <
        if(*start == '>')
        {
            if(*(start+1) == '>')
            {
                redir_type = APPEND_REDIR;
                // 追加重定向
                *start = '\0';
                start += 2;
                TrimSpace(start);
                filename = start;
                break;
            }
            else
            {
                // 输出重定向
                redir_type = OUPUT_REDIR;
                *start = '\0';
                start++;
                TrimSpace(start); // 移除空格
                filename = start;
                break;
            }
        }
        else if(*start == '<')
        {
            //输入重定向
            // cat <    log.txt
            redir_type = INPUT_REDIR;
            *start = '\0';
            start++;
            TrimSpace(start);
            filename = start;
            break;
        }
        else
        {
            start++;
        }
    }
}

// "ls -a -l"
bool ParseCommandString(char cmd[])
{
    if(cmd == NULL)
        return false;
#define SEP " "
    //3. "ls -a -l" -> "ls" "-a" "-l"
    gargv[gargc++] = strtok(cmd, SEP);
    // 整个数字，最后以NULL结尾
    while((bool)(gargv[gargc++] = strtok(NULL, SEP)));
    // 回退一次，命令行参数的格式
    gargc--;

//#define DEBUG
#ifdef DEBUG
        printf("gargc: %d\n", gargc);
        printf("----------------------\n");
        for(int i = 0; i < gargc; i++)
        {
            printf("gargv[%d]: %s\n",i, gargv[i]);
        }
        printf("----------------------\n");
        for(int i = 0; gargv[i]; i++)
        {
            printf("gargv[%d]: %s\n",i, gargv[i]);
        }
#endif

    return true;
}


void InitGlobal()
{
    gargc = 0;
    memset(gargv, 0, sizeof(gargv));
    filename.clear();
    redir_type = NONE_REDIR;
}

void ForkAndExec()
{
    pid_t id = fork();
    if(id < 0)
    {
        //for : XXXXX
        perror("fork"); // errno -> errstring
        return;
    }
    else if(id == 0)
    {
        if(redir_type == OUPUT_REDIR)
        {
            int output = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
            (void)output;
            dup2(output, 1);
        }
        else if(redir_type == INPUT_REDIR)
        {
            int input = open(filename.c_str(), O_RDONLY);
            (void)input;
            dup2(input, 0);
        }
        else if(redir_type == APPEND_REDIR)
        {
            int appendfd = open(filename.c_str(), O_CREAT | O_WRONLY | O_APPEND);
            (void)appendfd;
            dup2(appendfd, 1);
        }
        else{
            //Do Nothing
        }
        // 子进程打开文件了，也进行了重定向，execl*不影响历史打开的文件!
        //子进程
        execvp(gargv[0], gargv);
        exit(0);
    }
    else
    {
        //父进程
        int status = 0;
        pid_t rid = waitpid(id, &status, 0);
        if(rid > 0)
        {
            lastcode = WEXITSTATUS(status);
        }
    }
}

bool BuiltInCommandExec()
{
    //内建命令: 是shell自己执行的命令，如同shell执行一个自己的函数
    //gargv[0]
    std::string cmd = gargv[0];
    bool ret = false;
    if(cmd == "cd")
    {
        // build
        if(gargc == 2)
        {
            std::string target = gargv[1];
            if(target == "~")
            {
                ret = true;
                chdir(GetHomePath().c_str());
            }
            else{
                ret = true;
                chdir(gargv[1]);
            }
        }
        else if(gargc == 1)
        {
            ret = true;
            chdir(GetHomePath().c_str());
        }
        else
        {
            //BUG
        }
    }
    else if(cmd == "echo")
    {
        if(gargc == 2)
        {
            std::string args = gargv[1];
            if(args[0] == '$')
            {
                if(args[1] == '?')
                {
                    printf("lastcode: %d\n", lastcode);
                    lastcode = 0;
                    ret = true;
                }
                else{
                    const char *name = &args[1];
                    printf("%s\n", getenv(name));
                    lastcode = 0;
                    ret = true;
                }
            }
            else{
                printf("%s\n", args.c_str());
                ret = true;
            }
        }
    }
    return ret;
}