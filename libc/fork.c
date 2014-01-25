#include <stdlib.h>
#include <syscall.h>

uint64_t fork()
{
    //int pid = 0;
    return __syscall0(FORK);
    //return pid;
};