#include <stdarg.h>
#include <syscall.h>
#include <defs.h>

size_t strlen(const char* str){
    size_t ret = 0;
    while ( str[ret] != 0 )
        ret++;
    return ret;
}

char * itoa( uint64_t value, char * str, int base, int sign_flag ){
    char * ptr;
    char * low;
    char * high;

    if ( base < 2 || base > 36 ) {
        *str++ = '\0';
        return str;
    }
    ptr = str;

    if ( sign_flag ==1 && base == 10 ) {
        *ptr++ = '-';
    }
    if (base == 16) {
        *ptr++ = '0';
        *ptr++ = 'x';
    }
    low = ptr;
    do {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
        // Terminating the string.
    *ptr-- = '\0';

    high = ptr+1;
    // Invert the numbers.
    while ( low < ptr ) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return high;
}

int vsprintf(char *buf, const char *fmt, va_list args) {
    int base;
    char *str;
    const char *s;
    uint64_t num_uint64;
    long int num_int64;
    int i,len;

    for (str = buf; *fmt; ++fmt) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }

        ++fmt;

        base = 10;

        switch (*fmt) {
            case 'c':
                *str++ = (unsigned char)va_arg(args, int);
                continue;
            case 's':
                s = va_arg(args, char *);
                len = strlen(s);

                for (i = 0; i < len; ++i)
                    *str++ = *s++;
                continue;
            case 'p':
                str = itoa((unsigned long)va_arg(args, void *), str, 16, 0);
                continue;
            case 'x':
            case 'X':
                base = 16;
                num_uint64 = va_arg(args, uint32_t);
                str = itoa(num_uint64 ,str , base, 0);
                break;
            case 'd':
                num_int64 = va_arg(args, int);
                if(num_int64<0)
                	str = itoa(-num_int64, str, base, 1);
                else
                	str = itoa(num_int64, str, base, 0);
                break;
            case 'u':
                num_uint64 = va_arg(args, uint32_t);
                str = itoa(num_uint64 ,str , base, 0);
                break;
            case 'l':
                switch(*++fmt){
                case 'd':
                        num_int64 = va_arg(args, long int);
                        if(num_int64<0)
                                str = itoa(-num_int64, str, base, 1);
                        else
                        str = itoa(num_int64, str, base, 0);
                        break;
                case 'u':
                        num_uint64 = va_arg(args, uint64_t);
                        str = itoa(num_uint64 ,str , base, 0);
                        break;
                case 'x':
                        base = 16;
                        num_uint64 = va_arg(args, uint64_t);
                        str = itoa(num_uint64 ,str , base, 0);
                        break;

                }
        }
     }
     *str = '\0';
     return str - buf;
}

void puts(const char* data)
{
	//__syscall1(1,(uint64_t)data);
	//return
	 __syscall1(PRINTF,(uint64_t)data);
}

int printf(const char *format, ...) {
    char printf_buf[1024];
    va_list args;
    int printed;
    va_start(args, format);
    printed = vsprintf(printf_buf, format, args);
    va_end(args);
    puts(printf_buf);

    return printed;
}

 



