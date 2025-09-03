#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define ERR_EXIT(m) \
do\
{\
  perror(m);\
  exit(EXIT_FAILURE);\
}while(0)

int main()
{
  mkfifo("tp", 0666);
  int infd;
  infd = open("abc", O_RDONLY);

  if (infd == -1)
    ERR_EXIT("open");

  int outfd;
  outfd = open("tp", O_WRONLY);
  if(outfd==-1) ERR_EXIT("open");

  char buf[1024];
  int n;
  while((n=read(infd,buf,1024))>0)
  {
    write(outfd,buf,n);
  }
  close(infd);
  close(outfd);

  return 0;
}