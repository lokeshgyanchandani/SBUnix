#include<syscall.h>   
#include<sys/dirent.h> 

#define NOTFOUND 0

int closedir(DIR *dir)
{
  uint64_t a = (uint64_t)dir;
  uint64_t ret = __syscall1(CLOSEDIR,a);
  return ret;
 
 }