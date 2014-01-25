#include<syscall.h>


void wait()
{
	 __syscall0(WAIT);
}

void waitpid(int pid)
{
	 __syscall1(WAITPID,pid);
}

