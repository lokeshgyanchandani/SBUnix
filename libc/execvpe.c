#include<syscall.h>
#include<stdlib.h>

int execve(char *filename,char *argv[],char *env[])
{
    uint64_t a = (uint64_t)argv;
    int rows = 0;
    int ret = 0;
    ret = __syscall3(EXECVE,(uint64_t)filename, a, rows);
    return ret;
}
