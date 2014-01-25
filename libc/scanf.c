#include<stdio.h>                                                           
#include<defs.h>
#include<syscall.h>
#include <stdlib.h>
#include <math.h>

#define va_start(v,l)     __builtin_va_start(v,l)
#define va_arg(v,l)       __builtin_va_arg(v,l)
#define va_end(v)         __builtin_va_end(v)
#define va_copy(d,s)      __builtin_va_copy(d,s)

int atoi(char *nptr);
uint64_t atox(const char * s);


int read(int fd, char* buffer,int size)
 {
   uint64_t a = fd;
   uint64_t b = (uint64_t)buffer;
   uint64_t c = size;
   int ret;
   ret =  __syscall3(READ,a,b,c);
   return ret;
}

void read1(char *line,int size)
   {
    do
     {
      read(0,line,100);
      }while(*line == '\0');
  }


int scanf(const char * format,...)
{
  char input[10]={0};
  typedef __builtin_va_list va_list;
  va_list scan;
  va_start(scan,format);
  while(*format)
  {
    if(*format == '%')
      switch(*++format)
      {
        case 'd':;
                 		   
                 read1(input,sizeof(int));
		int *num = va_arg(scan,int*);

		  int k = 0;
		  int i=0;
		    	while (input[i]) {
	       	 k = (k<<3)+(k<<1)+(input[i])-'0';
	       	 i++;
			    }
		   *num  = k;
		   char *s = "";
		   printf("%s",s);
                 break;
        case 's':;
                 char *str = va_arg(scan,char*);
                 read1(str,10);
		  char *s1 = "";
		   printf("%s",s1);

                 break;
        case 'x':;
                 
                 read1(input,sizeof(int));
		  uint64_t *num1 = va_arg(scan,uint64_t*);
                 *num1 =atox(input);          
          default:break;
      }
  format++;
  }

  return 0;
}

int isdigit(char c)
{
int a = c - 48;
if(a>=0 && a<=9)
 return 1;
else
  return 0;
}


int atoi(char *nptr)
{
    int base=1;
    int res=0;
    int i;

          
    for (i=0; *(nptr+i); ++i) {
      if(!isdigit(*(nptr+i)))
        return -1;
     }

    i=0;
    while(nptr[++i])
       base *= 10;

    for (i=0; *(nptr+i); ++i ) {
       res += ((int)nptr[i] - 48) * base;
       base /= 10;
     }

  return res;
}


uint64_t atox(const char * s) {
uint64_t result = 0;
int c ;
if ('0' == *s && 'x' == *(s+1)) { s+=2;
  while (*s) {
    result = result << 4;
    if (c=(*s-'0'),(c>=0 && c <=9)) result|=c;
    else if (c=(*s-'A'),(c>=0 && c <=5)) result|=(c+10);
    else if (c=(*s-'a'),(c>=0 && c <=5)) result|=(c+10);
    else break;
    ++s;
    }
  }
      return result;
}

