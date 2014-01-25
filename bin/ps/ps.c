#include <syscall.h>

#include<stdio.h>
int main()
{
    printf("\n");
    __syscall0(LISTPROCESS);
     return 0;
}

