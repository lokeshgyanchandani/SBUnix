#include<syscall.h>   
#include<sys/dirent.h> 
#include<stdio.h>

#define NOTFOUND 0

dirent * readdir(DIR* dir)
{
  uint64_t a = (uint64_t)dir;
  dirent * dent = (dirent *)malloc(sizeof(dirent));
  uint64_t b = (uint64_t)dent;
  uint64_t ret= __syscall2(READDIR,a,b);
  if(ret != NOTFOUND){
		return (dirent *)b;
  }
  return NULL;
}