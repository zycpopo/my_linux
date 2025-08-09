#ifndef __MYSHELL_H__
#define __MYSHELL_H__

#include<stdio.h>

#define ARGS 64

void Debug();
void InitGlobal();
void PrintCommandPrompt();
bool GetCommandString(char cmd_str_buff[],int len);
void CheckRedir(char cmd[]);
bool ParseCommandString(char cmd[]);
void ForkAndExec();
bool BuiltInCommandExec();

#endif