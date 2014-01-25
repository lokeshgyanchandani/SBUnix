#include <stdio.h>
#include <syscall.h>

int main(int argc, char* argv[]) {

	printf("\n!!  Hello - I am inside user process  !!\n");

	printf("\nTab:\t %s, %c", "testing", ':');

	return 0;
}
