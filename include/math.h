#ifndef _MATH_H
#define _MATH_H

#include <defs.h>

//int strcmp(char *s1, char *s2);
int pow(int x, int y);
int getSize(char *p) ;

char * strcpy(char *dst, const char *src);
char * strncpy(char *dst, const char *src, size_t size);
size_t strlcpy(char *dst, const char *src, size_t size);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t size);
char * strchr(const char *s, char c);
char * strfind(const char *s, char c);

int lastIndexOf (const char* base, const char* str);
int indexOf_shift(const char* base, const char* str, int startIndex);
//int strlen(const char *str);
int starts_with(const char * base, const char *prefix);
char *substring(char* dest , const char *s, int startIndex, int endIndex);
char *strstr(const char *haystack, const char *needle);

int parse(char *line, char **argv);

#endif
