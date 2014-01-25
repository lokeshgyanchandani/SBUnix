#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>

int main(int argc, char* argv[])
{
	printf("\nProcess says %s\n\n", "FORK");

	int pid = fork();

	if(pid > 0)
	{
		//waitpid(pid);

		sleep(10);

		printf("\nParent ProcessID : %d\n", pid);

		//sleep(10);
		exit(0);
	}
	else
	{
		printf("\nChild ProcessID : %d\n", pid);
		printf("SBU ");
		exit(0);
	}
	return 0;
}

