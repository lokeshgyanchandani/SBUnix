#include <sys/io.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/pagetab.h>
#include <math.h>
#include <sys/file.h>
#include <syscall.h>

#include <defs.h>
#include <sys/kbc.h>
#include <sys/kbc_scancodes.h>



#define START_ADDR_PRINTF 0xB8000
#define BOBCOLOR 0x0F	//black background & black base
#define DOSCOLOR 0x1F	//DOS back color
#define BSPACE 0x20		//ASCII value for space
#define ROW 25
#define COL 80

//******global coordinates for tracking the cursor
int c_x = 0;
int c_y = 0;

char *start_addr_printf = (char *)START_ADDR_PRINTF;

char val[64];

void clrscr();
void update_cursor();
void update_cursor_no_type();


void sys_write(uint64_t rbx,uint64_t rcx,uint64_t rdx)
 {
   int fd = rbx;
   char *str =(char*)rcx;
  long size = rdx;
   if (fd == 1 || fd == 2 )
   {
     char *temp = str;
   while((size--)> 0 && *str++);
   *str = '\0';
     puts(temp);
   }
 }


int shiftPressed = 0;
int ctrlPressed = 0;



char buf[256]="\0";
char *buffer = buf;
int isNewline =0;



void kbc_irq()
{
    static uint8_t shift = 0;

    uint8_t b = inb(0x60);

    switch(b)
    {
        case KBCSC_LEFTSHIFT:
            shift = 1;
            break;
        case KBCSC_RIGHTSHIFT:
            shift = 2;
            break;
        case (KBCSC_LEFTSHIFT | 0x80):
            shift = 1;
            break;
        case (KBCSC_RIGHTSHIFT | 0x80):
            shift = 2;
	     break;
        default:
            break;
    }




    uint8_t c = kbcsc_tochar(b, shift);
    if(c != 0)
    {
		
		if(c != (char)'\b')
		{
			*buffer++ = c;
		}

		if(c == (char)'\n')
			isNewline = 1;

		if(c == (char)'\b' && (buffer - buf) > 0 )
		{
			*(--buffer) = '\0';
		}	

        print_cx_cy(52,24,"%c", c);
        printf("%c", c);		//for echo
    }

}


char temp[150]={0};

char* gets()
{
    int i=0,j=0;
    int k=0;

    for(k=0;k<150;k++)
        temp[k] = '\0';
   
    if(!isNewline)
        return NULL;
    else
    {
        while(buf[i]!='\n')
        {
            temp[i]=buf[i];
            i++;
        }

        isNewline =0;
        temp[i++]='\0';
    }

    while(buf[i])
    {
        buf[j++]=buffer[i++];
    }
 
    for(i=j;i<256;i++)
        buf[i] = '\0';

    buffer = &buf[j];

    return temp;
}


int sys_read(uint64_t rbx,uint64_t rcx,uint64_t rdx)
{
/*
    char *str = (char*)rbx;
    char *str1 = gets();
  
    if(str1 == NULL)
        *str = '\0';
    else
    {

        while(*str1)
        *str++ = *str1++;
        *str = '\0';
*/
   	int fd = rbx;
   	char *str = (char*)rcx;
   	int size = rdx;
   	int readBytes = 0;
   	if(fd < 3)
	{
	  char *str1 = gets();
	  if(str1 == 0)
	  {
		*str = '\0';
	  }
	  else
	  {
	   while(*str1)
          {
		 *str++ = *str1++;
		 readBytes++;
 	   }
	   *str = '\0';
	   readBytes++;
	  }
	}else
	{
		readBytes = t_read(fd, str, size);
	}
	return readBytes;
 //   }
}






int printf(const char *format, ...)
{
    const char *str;
    char *asciivalue;
    va_list params;

    int total = 0;
    va_start(params, format);
    str = format;
    while (*str != 0)
    {
	 if (*str != '%')
	 {
	     total += putchar(*str++);
	     continue;
	 }
	 str++;
	 switch(*str)
	 {
	     case 'c':
		  total += putchar(va_arg(params, int));
		  break;
	     case 'd':
		  asciivalue = itoa(va_arg(params, int), val, 10);
		  total += puts(asciivalue);
		  break;
	     case 'x':
                asciivalue = itoa(va_arg(params, int), val, 16);
		  total += puts(asciivalue);
		  break;
	     case 's':
		  total += puts(va_arg(params, char *));
		  break;
	     case 'p':
		  asciivalue = itoa(va_arg(params, unsigned long int), val, 16);
		  total += puts(asciivalue);
		  break;
	     default:
		  total += putchar(*str);
		  break;
	 }
	 str++;
    }
    va_end (params);
    return total;
}

int putchar(int ch)
{
    char c = ch;
    if (c == '\n')
    {
	 c_x = 0;
	 c_y++;
	 if (c_y == ROW - 1)
	 {
	     c_y = 0;
	 }

	 update_cursor_no_type();
    }
    else if (c == '\b')
    {
	 if(c_x >= 1)
        {
	     c_x--;
	 }

        update_cursor_no_type();
    }
    else if (c == '\t')
    {
	 int nextStop = (c_x + 8); //each \t is 8 spaces long
	 if(nextStop <= COL)
        {
	     c_x = nextStop;
	 }
        else
        {
	     c_x = nextStop % COL;
	     c_y++;
	     if (c_y == ROW - 1)
	     {
		  c_y = 0;
	     }
	 }

	 update_cursor_no_type();
    }
    else
    {
        //volatile char * current_addr = (volatile char *)((uint64_t)0xffffffff8020c000) + (c_y * 2 * 80 + c_x * 2);
	 volatile char * current_addr =
	    (volatile char *)start_addr_printf + (c_y * 2 * 80 + c_x * 2);
	 *current_addr++ = c;
	 *current_addr++ = BOBCOLOR;
	 update_cursor();
    }

    return ch;
}

int puts(const char *string)
{
	while(*string != '\0')
	{
		putchar(*string);
		string++;
	}
	return 0;
}


char * itoa( uint64_t value, char * str, uint64_t base )
{
    char * rc;
    char * ptr;
    char * low;
    int count = 0;

    // Check for supported base.
    if ( base < 2 || base > 16 )
    {
        *str = '\0';
        return str;
    }

    rc = ptr = str;
    // Set '-' for negative decimals.
    if ( value < 0 && base == 10 )
    {
	 count++;
        *ptr++ = '-';
    }

    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
	 count++;
	 int d = value % base;
	 value /= base;
	 if(d <= 9)
	     *ptr++ = (char)d + '0';
	 else
	     *ptr++ = (char)d + 'a'-10;
    } while ( value );
    while(count++ < 16 && base == 16)
        *ptr++ = '0';

    // Terminating the string.
    *ptr-- = '\0';	

    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

int atoi(char *p) {
    int k = 0;

    while (*p) {
        k = (k<<3)+(k<<1)+(*p)-'0';
        p++;
    }
    return k;
}

int octal_decimal(int n) {
    int decimal=0, i=0, rem; 
 
    while (n!=0) { 
        rem = n%10; 
        n/=10; 
        decimal += rem * pow(8,i);
        ++i; 
    }
    return decimal;
}

void clrscr()
{
    int index = 0;
    c_x = 0;
    c_y = 0;
    while(index < (ROW - 1) * (COL - 1))
    {
        putchar(' ');
	 index++;
    }
    c_x = 0;
    c_y = 0;

    update_cursor_no_type();
}

void update_cursor()
{
    c_x ++;
    if (c_x == COL)
    {
        c_x = 0;
	 c_y++;
	 if (c_y == ROW - 1)
	 {
	     c_y = 0;
	 }
    }

    update_cursor_no_type();
}

void update_cursor_no_type()
{
    /*unsigned short position = (c_y * 80 + c_x);

    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));

    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char )((position >> 8) & 0xFF));*/
}

//##############################################Print functions for kb and clock timer######################################3

int c_timer_x = 0;
int c_timer_y = 0;

int putchar_cx_cy(int ch)
{
    char c = ch;
    volatile char * current_addr =
	 (volatile char *)start_addr_printf + (c_timer_y * 2 * 80 + c_timer_x * 2);
    *current_addr++ = c;
    *current_addr++ = DOSCOLOR;	
    c_timer_x++;
    return ch;
}

int puts_cx_cy(const char *string)
{
    while(*string != '\0')
    {
        putchar_cx_cy(*string);
        string++;
    }
    return 0;
}

//print_timer function for printing timer at the bottom right of the clock
int print_cx_cy(int row, int col, const char *format, ...)
{
    const char *str;
    char *asciivalue;
    va_list params;

    c_timer_x = row;
    c_timer_y = col;
    int total = 0;
    va_start(params, format);
    str = format;
    while (*str != 0)
    {
	 if (*str != '%')
	 {
	      total += putchar_cx_cy(*str++);
             continue;
	 }
	 str++;
	 switch(*str)
	 {
	     case 'c':
	         total += putchar_cx_cy(va_arg(params, int));
		  break;
	     case 'd':
		  asciivalue = itoa(va_arg(params, int), val, 10);
		  total += puts_cx_cy(asciivalue);
		  break;
	     case 'x':
		  asciivalue = itoa(va_arg(params, int), val, 16);
		  total += puts_cx_cy(asciivalue);
	         break;
	     case 's':
                total += puts_cx_cy(va_arg(params, char *));
		  break;
	     case 'p':
		  asciivalue = itoa(va_arg(params, unsigned long int), val, 16);
                total += puts_cx_cy(asciivalue);
		  break;
	     default:
		  total += putchar_cx_cy(*str);
		  break;
        }
	 str++;
    }
    va_end (params);
    return total;
}

void remap_printf()
{
	start_addr_printf = PHY_TO_VIRT(START_ADDR_PRINTF, char *);
}

//##############################################################END######################################


void *memcpy(void *dest, const void *src, size_t count)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    for(; count != 0; count--) *dp++ = *sp++;
    return dest;
}

// Copies 8 bytes at a time
void *memcpy8(void *destination, void *source, uint64_t num) 
{
    uint64_t *dest = (uint64_t *)destination;
    uint64_t *src = (uint64_t *)source;

    while(num--) {
        *dest++ = *src++; 
    }

    return destination;
}

void *memset(void *dest, char val, size_t count)
{
    char *temp = (char *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

size_t strlen(const char *str)
{
    size_t len;
    for(len = 0; *str != '\0'; str++) len++;
    return len;
}