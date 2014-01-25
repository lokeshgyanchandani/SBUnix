
#include <stdio.h>
#include <stdlib.h>
#include<syscall.h>

int main(int argc, char* argv[]) {

	const char* path= "etc/new.txt";
	printf("\nAddress of string %x\n",path);
	printf("\nFiles opened with descriptor %d, %d", open(path,8), open("bin/hello",7));
	DIR * directory = opendir("etc/");
	printf("\etc/- %s", readdir(directory)->d_name);
	return 0;
}