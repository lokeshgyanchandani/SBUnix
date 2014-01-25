#ifndef IO_H
#define IO_H

#include <defs.h>

static __inline void io_wait()
{
    __asm__(
	 "jmp 1f\n\t"
        "1:jmp 2f\n\t"
        "2:" 
       	);
}

static __inline unsigned char inb( unsigned short port )
{
    unsigned char ret;
    __asm__(
	 "inb %1, %0;"
        : "=a"(ret)
	 : "Nd"(port) 
	);
    return ret;
}

static __inline void outb( unsigned short port, unsigned char val )
{
	__asm__(
		"outb %0, %1;"
	  	:
	  	: "a"(val), "Nd"(port)
	 );
}


#endif
