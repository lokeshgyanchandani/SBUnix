#ifndef _STDIO_H
#define _STDIO_H

#include <defs.h>
#include <sys/dirent.h>
#include <syscall.h>


int printf(const char *format, ...);
int print_cx_cy(int row, int col, const char *format, ...);
int scanf(const char *format, ...);
//void scanf(char*,void*);
//void scank(char*,char*);
int puts(const char * string);
int puts_cx_cy(const char *string);
int putchar(int ch);
int putchar_cx_cy(int ch);

void clrscr();
char * itoa(uint64_t value, char *str, uint64_t base);
int atoi(char *p);
int octal_decimal(int n);

void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *dest, char val, size_t count);

void kscanf(const char *str, ...);

// Copies 8 bytes at a time
void *memcpy8(void *destination, void *source, uint64_t num);
void *malloc(uint64_t no_bytes);


size_t strlen(const char *str);

void remap_printf();

#endif
