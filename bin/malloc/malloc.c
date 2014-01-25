#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

	printf("Inside malloc\n\n");

	char* test_malloc = (char*)malloc(4);
	test_malloc = "abc";

	printf("hey %p ->> %s\n",test_malloc,test_malloc);

	while(1);

	return 0;
}
