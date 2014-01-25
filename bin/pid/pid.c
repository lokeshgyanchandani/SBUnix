#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	int s = get_pid();
	printf("PID :%d\n",s);
	return 0;
}
