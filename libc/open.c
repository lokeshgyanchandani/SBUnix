#include<syscall.h>    
int open(const char *pathname, int flags)
{
  uint64_t a = (uint64_t)pathname;
  uint64_t b = flags;
  int ret= __syscall2(OPEN,a,b);
  return ret;
}
