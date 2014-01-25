#include<syscall.h>   
#include<sys/dirent.h> 
#include<stdio.h>
#include<math.h>
#include <stdlib.h>
#include <sys/file.h>

int strcmp(const char *p, const char *q) {
	while (*p || *q) {
		if (*p != *q)
			return -1;
		p++, q++;
	}
	return 0;
}

char *substring(char* dest , const char *s, int startIndex, int endIndex)
{
  //char* result = dest;
  /* check for null s */
  if (NULL == s)
    return NULL;

  if (startIndex > endIndex)
    return NULL;
	
 
  /* n < 0 or m < 0 is invalid */
  if (startIndex < 0 || endIndex < 0)
    return NULL;
	
  int i = 0;
  for ( ; endIndex-startIndex > 0; startIndex++, i++)
    if (*(s+startIndex) != '\0'){
	  *(dest+i) = *(s+startIndex);	
	}else{
	  break;
	}
  *(dest+i) = 0;
  return dest;
}

int main(int argc, char* argv[])
{
    DIR *dir;
    struct dirent *dirfile;

	argv[1] = "";

	if(strcmp(argv[1],"") == 0)
	{
		char *path = malloc(100);
		path = "/bin";
		getPath(path);

		int len = strlen(path+1);
		//printf("\nlen%d",len);
		path[len] = '\0';
		if(len > 1)
		printf("Current Path :%s\n", (path+1));
		else
		printf("Current Path :/");

		dir = opendir(path+1);
	}
	else
	{	char *path = argv[1];
		if(*path == '/')
			path = path+1;
		dir = opendir(path);
	}
    if(dir != NULL)
    {
    	int dirLen = strlen(dir->dirname);
	printf("\nDirectories :");
	    while((dirfile = readdir(dir)) != NULL)
    	{
		 char dirc[100] = "";
       	 substring(dirc,dirfile->d_name,dirLen,strlen(dirfile->d_name));
	        printf("\n%s", dirc);
    	}
    	closedir(dir);
    }
    else
    {
   	printf("\n%s : directory not found.",argv[1]);
    }

	return 0;
}