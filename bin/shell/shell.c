#include <stdlib.h>
#include <bash.h>
#include <stdio.h> 
#include <defs.h>
#include <syscall.h>
#include <stdlib.h>
#include <sys/file.h>

#define BUFFER 20

int strln(char *st);
char *strcppy(char *dst, char *src);
void clear(char *pt, uint32_t len);
int strcmp(const char *p, const char *q);
int starts_with(const char * base, const char *prefix);
int parse(char *line, char **argv);

char *strcppy(char *dst, char *src)
{
	char *ret;
	ret = dst;
	while ((*dst++ = *src++) != '\0');
	return ret;
}


void readcommand(char *line)
{
	do
	{
		read(0, line, 100);
	}
	while(*line == '\0');
}

int main(int argc, char* argv[])
{
	char *buff = (char *) malloc(sizeof(char) * BUFFER);
	char *tempPtr = (char *) malloc(sizeof(char) * BUFFER);
	char *cur_PATH = (char *) malloc(sizeof(char) * 100);
	char *PATH = (char *) malloc(sizeof(char) * 100);
	char* argvv[5];
	int i;
	for (i = 0; i<5; i++)
		argvv[i] = malloc(500);

	strcppy(PATH, "/bin/ls");

	clear(buff, BUFFER);

	printf("Inside SHELL:: (supported commands --> 'ls', 'ps', 'cd')\n");

	while(1)
	{
		printf("\n%s$ ", cur_PATH);

		di();
		readcommand(buff);
		si();

		strcppy(tempPtr, buff);

		int fd = open(tempPtr, 7);

		if (fd > 2)
		{
			int shebang = 0;
			char *temp1 = malloc(3);
			strcppy(temp1, "");

			//int readbytes = read(fd, temp1, 2);
			int readbytes = 0;

			if (strcmp("#!", temp1) == 0)
				shebang = 1;

			if (shebang == 1)
			{
				char executable[50] = "";
				char a = '\0';
				char *ptr = &a;
				char *exeptr = executable;
				do
				{
					*ptr = '\0'; 
					readbytes = read(fd, ptr, 1);
					*exeptr ++ = a;
				} while ((readbytes != EOF) && a != '\n');

				printf("%s", executable);

				if(starts_with(executable,"/bin/sh") == 1)
				{
					char *args[] = {"         ", "          ", "          ","           "};

					while((readbytes != EOF))
					{
						char line[50] = "";
						char a ='\0';
						char *ptr = &a;
						char* linePtr = line;
						do
						{
							*ptr = '\0';

							readbytes = read(fd,ptr,1);
							printf("%c",a);
							*linePtr ++ = a;

						} while((readbytes != EOF) && a != '\n' );

						parse(line,args);
						if(strcmp(args[0],"cd")== 0)
	    					{
							printf("Changing directory to %s",args[1]);
							updatePath(args[1]);
				    		}

	    					else if( strcmp(args[0],"setenv")== 0)
	    					{
							updateEnvPath(args[1]);
				    		}
	    					else
				    		{
							int pid = fork();

							if(pid > 0)
							{
								waitpid(pid);
							}
							else
							{
								int res = execve(buff, argv, 0);

								if(res == -1)
								{
									printf("\n%s : Command not found!!!!",argv[0]);
									exit(0);
								}
							}
            					}
	   				}
				}
				close(fd);
				continue;
			}
		}

		switch(buff[0])
		{
			case '\0':
				printf("\n%s$ ", cur_PATH);
				break;
			default:

				parse(buff, argvv);

				if(strcmp(argvv[0],"cd")== 0)
				{
					printf("Changing directory to %s",argvv[1]);
					updatePath(argvv[1]);
				}

				else if(strcmp(argvv[0],"setenv")== 0)
				{
					updateEnvPath(argvv[1]);
				}

				else if (strcmp(buff, "ls") == 0 || strcmp(buff, "ps") == 0 || starts_with(buff, "bin/"))
				{

					if (strcmp(buff, "ls") == 0)
					{
						clear(buff, BUFFER);
						strcppy(buff, "bin/ls");
					}
					else if (strcmp(buff, "ps") == 0)
					{
						clear(buff, BUFFER);
						strcppy(buff, "bin/ps");
					}

					int pid = fork();

					if(pid > 0)
					{
						waitpid(pid);
					}
					else
					{
						int res = execve(buff, argv, 0);

						if(res == -1)
						{
							printf("\nErrr!: Command not found!\n\n");
						}
						exit(0);
					}
				}

				else
				{
					printf("\nErrr!: Command not found!\n\n");
				}

				break;
		}
		clear(buff, BUFFER);
	}
	return 0;
}	


int parse(char *line, char **argv)
{ 
	int len =0;

	while (*line != '\0') {
		len++;
		while (*line == ' ' || *line == '\t' || *line == '\n')
			*line++ = '\0';
		*argv++ = line;
		while (*line != '\0' && *line != ' ' && 
			*line != '\t' && *line != '\n') 
		line++;
	}
	*argv = '\0';
	return len;
}

int strln(char *st)
{
	uint32_t i = 0;

	while ((*st != '\0') && (st != NULL))
	{
		++st;
		++i;
	}

	return (i);
}

int strcmp(const char *p, const char *q) {
	while (*p || *q) {
		if (*p != *q)
			return -1;
		p++, q++;
	}
	return 0;
}

char *strcpy(char *dst, const char *src) {
	char *ret;
	ret = dst;
	while ((*dst++ = *src++) != '\0');
	return ret;
}

int starts_with(const char * base, const char *prefix)
{
    while(*prefix)
    {
        if(*prefix++ != *base++)
            return 0;
    }
    return 1;
}

void clear(char *pt, uint32_t len)
{
	uint32_t i = 0;

	while(i < len)
	{
		pt[i] = 0;
		++i;
	}
}