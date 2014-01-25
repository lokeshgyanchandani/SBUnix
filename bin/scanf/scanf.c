#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	//int i=1;
	char i[100];
	i[0] = 's';
	i[1] = 'a';
	printf("the value of i :%s",i);
	scanf("%s",i);
	printf("the value of i :%s",i);
	//scanf("%s",i);
	//printf("the value of i :%s",i);
	//while(1);
	//__syscall0(EXIT);
	exit(0);
	return 0;
}
