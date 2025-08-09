#include "myshell.h"

#define SIZE 1024

int main()
{
  char commandstr[SIZE];
  while(true)
  {
    //0.初始化操作
    InitGlobal();
    //1.输出命令行提示符
    PrintCommandPrompt();
    //2.获取用户输入的命令
    if(!GetCommandString(commandstr,SIZE))
        continue;
    //3.重定向检测
    CheckRedir(commandstr);
    //4.对命令字符串解析
    ParseCommandString(commandstr);
    //5.检测命令，判断内建命令
    if(BuiltInCommandExec())
    {
      continue;
    }
    //6.执行命令
    ForkAndExec();
  }

  return 0;
}