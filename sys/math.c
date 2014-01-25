#include <defs.h>
#include <math.h>
#include <stdio.h>

char *strcpy(char *dst, const char *src) {
	char *ret;
	ret = dst;
	while ((*dst++ = *src++) != '\0');
	return ret;
}

char *strcat(char *dst, const char *src) {
	int len = strlen(dst);
	strcpy(dst + len, src);
	return dst;
}

char *strtrim_start(char *dst, int trim) {
   return dst + trim;
}

char *strncpy(char *dst, const char *src, size_t size) {
	size_t i;
	char *ret;
	ret = dst;
	for (i = 0; i < size; i++) {
		*dst++ = *src;
		if (*src != '\0')
			src++;
	}
	return ret;
}

size_t strlcpy(char *dst, const char *src, size_t size) {
	char *dst_in;
	dst_in = dst;
	if (size > 0) {
		while (--size > 0 && *src != '\0')
			*dst++ = *src++;
		*dst = '\0';
	}
	return dst - dst_in;
}


/*
int strcmp(char *s1, char *s2)
{
       while (*s1 == *s2++) {
          if (*s1++ == 0){ 
               return (0);
                 }
            }
        return (*(unsigned char *) s1 - *(unsigned char *) --s2);
}
*/


int strcmp(const char *p, const char *q) {
	while (*p || *q) {
		if (*p != *q)
			return -1;
		p++, q++;
	}
	return 0;
}

int strncmp(const char *p, const char *q, size_t n) {
	while (n > 0 && *p && *p == *q)
		n--, p++, q++;
	if (n == 0)
		return 0;
	else
		return (int) ((unsigned char) *p - (unsigned char) *q);
}

// Return a pointer to the first occurrence of 'c' in 's',
// or a null pointer if the string has no 'c'.
char *strchr(const char *s, char c) {
	for (; *s; s++)
		if (*s == c)
			return (char *) s;
	return 0;
}

// Return a pointer to the first occurrence of 'c' in 's',
// or a pointer to the string-ending null character if the string has no 'c'.
char *strfind(const char *s, char c) {
	for (; *s; s++)
		if (*s == c)
			break;
	return (char *) s;
}




int pow(int x, int y)
{
      if( y == 0)
                return 1;
          else if (y%2 == 0)
                    return pow(x, y/2)*pow(x, y/2);
              else
                        return x*pow(x, y/2)*pow(x, y/2);
               
}

int getSize(char *p) 
{
   int k = 0;
    while (*p) {
       k = (k<<3)+(k<<1)+(*p)-'0';
        p++;
         }

     int decimal=0, i=0, rem; 
      while (k!=0) 
         { 
             rem = k%10; 
               k/=10; 
                 decimal += rem*pow(8,i); 
                   ++i; 
                    } 
       return decimal; 
}

/*
int strlen(const char *str)
{
        const char *s;

        for (s = str; *s; ++s)
                ;
        return (s - str);
}
*/

int indexOf_shift(const char* base, const char* str, int startIndex) {
        int result;
        int baselen = strlen(base);
        // str should not longer than base
        if (strlen(str) > baselen || startIndex > baselen) {
                result = -1;
        } else {
                if (startIndex < 0 ) {
                        startIndex = 0;
                }
                char* pos = strstr(base+startIndex, str);
                if (pos == NULL) {
                        result = -1;
                } else {
                        result = pos - base;
                }
        }
        return result;
}

int lastIndexOf (const char* base, const char* str) {
        int result;
        // str should not longer than base
        if (strlen(str) > strlen(base)) {
                result = -1;
        } else {
                int start = 0;
                int endinit = strlen(base) - strlen(str);
                int end = endinit;
                int endtmp = endinit;
                while(start != end) {
                        start = indexOf_shift(base, str, start);
                        end = indexOf_shift(base, str, end);

                        // not found from start
                        if (start == -1) {
                                end = -1; // then break;
                        } else if (end == -1) {
                                // found from start
                                // but not found from end
                                // move end to middle
                                if (endtmp == (start+1)) {
                                        end = start; // then break;
                                } else {
                                        end = endtmp - (endtmp - start) / 2;
                                        if (end <= start) {
                                                end = start+1;
                                        }
                                        endtmp = end;
                                }
                        } else {
                                // found from both start and end
                                // move start to end and
                                // move end to base - strlen(str)
                                start = end;
                                end = endinit;
                        }
                }
                result = start;
        }
        return result;
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

int starts_with(const char * base, const char *prefix) //returns 1 if match is found
{
    while(*prefix)
    {
        if(*prefix++ != *base++)
            return 0;
    }
    return 1;
}

char *strstr(const char *haystack, const char *needle)
 {
     int needlelen;
     /* Check for the null needle case.  */
     if (*needle == '\0')
         return (char *) haystack;
     needlelen = strlen(needle);
     for (; (haystack = strchr(haystack, *needle)) != NULL; haystack++)
         if (strncmp(haystack, needle, needlelen) == 0)
             return (char *) haystack;
     return NULL;
 }

int parse(char *line, char **argv)
{ 
  int len =0;
 
     while (*line != '\0') {       /* if not the end of line ....... */ 
       len++;
       while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n') 
               line++;             /* skip the argument until ...    */
     }
     *argv = '\0';                 /* mark the end of argument list  */
return len;
}
