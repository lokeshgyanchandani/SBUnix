#include <stdio.h>
#include<syscall.h>   
#include<sys/dirent.h> 

#define NOTFOUND 0

DIR * opendir(const char *fullpath)
{
  uint64_t a = (uint64_t)fullpath;
  DIR * dir = (DIR *)malloc(sizeof(DIR));
  uint64_t b = (uint64_t)dir;
  uint64_t ret = __syscall2(OPENDIR,a,b);
  if(ret != NOTFOUND){
		return (DIR*)b;
  }
  return NULL;
}
