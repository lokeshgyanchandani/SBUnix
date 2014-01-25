#include <syscall.h>

void sleep(int seconds)
{
	__syscall1(SLEEP, seconds);
}