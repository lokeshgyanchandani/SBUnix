#include <syscall.h>

void exit(int status) {
	__syscall0(EXIT);
}

void di()
{
	__syscall0(DISABLE_INTERRUPT);
}

void si()
{
	__syscall0(ENABLE_INTERRUPT);
}