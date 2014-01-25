#include <syscall.h>
#include <stdlib.h>

int get_pid() {
	return __syscall0(GETPID);	//GET ID OF THE PROCESS
}

int get_ppid() {
	return __syscall0(GETPPID);  //GET PARENT ID OF THE PROCESS
}

void list_process() {
	__syscall0(LISTPROCESS);
}