#include<syscall.h>    

int write(int fd,char* buffer,int size)
{

  uint64_t a = fd;
  uint64_t b = (uint64_t)buffer;
  uint64_t c = size;
  int ret=0;

  __syscall3(WRITE,a,b,c);
  return ret;
}
