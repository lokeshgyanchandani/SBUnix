#include<syscall.h>
#include <stdlib.h>

void getPath(char* buffer)                                             
  {
    uint64_t a = (uint64_t)buffer;
    __syscall1(CURRENTPATH,a);
 }

void updatePath(char* buffer)                                             
  {
    uint64_t a = (uint64_t)buffer;
    __syscall1(UPDATEPATH,a);
 }


void updateEnvPath(char* buffer)
  {
    uint64_t a = (uint64_t)buffer;
    __syscall1(UPDATE_ENV,a);
 }
