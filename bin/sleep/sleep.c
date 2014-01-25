#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>

int main(int argc, char* argv[]) 
{
	int sleepVal = atoi( argv[1]);

	if(sleepVal >0)
	{
		sleep(sleepVal );
	}

	return 0;
}