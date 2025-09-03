#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define ERR_EXIT(m)     \
  do                    \
  {                     \
    perror(m);          \
    exit(EXIT_FAILURE); \
  } while (0);

int main()
{
  int outfd;
  outfd = open("abc.bak", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (outfd == -1)
    ERR_EXIT("open");

  int infd;
  infd = open("tp", O_RDONLY);
  if (outfd == -1)
    ERR_EXIT("open");

  char buf[1024];
  int n;
  while ((n = read(infd, buf, 1024)) > 0)
  {
    write(outfd, buf, n);
  }

  close(outfd);
  close(infd);
  unlink("tp");

  return 0;
}