#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <defs.h>

#define SYSCALL_PROTO(n) static __inline uint64_t __syscall##n


SYSCALL_PROTO(0)(uint64_t n) {
	/*uint64_t __res;
	__asm__ volatile ("int $0x80" \
       	           : "=a" (__res) \
			    : "0" (n));
	return __res;*/
     uint64_t ret;
    __asm__ volatile("movq %1,%%rax;"
                   "int $0x80;"
                   "movq %%rax,%0;"
                   :"=r"(ret)
                   :"r"(n)
                   :"rax"
                   );
           return ret;
};

SYSCALL_PROTO(1)(uint64_t n, uint64_t arg1) {
	uint64_t __res;
	__asm__ volatile ("int $0x80" \
       	           : "=a" (__res) \
			    : "0" (n) ,"b"((uint64_t)(arg1)));
	return __res;
}

SYSCALL_PROTO(2)(uint64_t n, uint64_t arg1, uint64_t arg2) {
	/*uint64_t __res;
	__asm__ volatile ("int $0x80" \
	                  : "=a" (__res) \
			    : "0" (n) ,"b"((uint64_t)(arg1)),"c"((uint64_t)(arg2)));
	return __res;*/
	   uint64_t ret;
	   __asm volatile("movq %1,%%rax;"
                  "movq %2, %%rbx;"
		    "movq %3, %%rcx;"
                  "int $0x80;"
                  "movq %%rax,%0;"
                  :"=r"(ret)
                  :"r"(n),"r"(arg1),"r"(arg2)
                  :"rax","rbx","rcx"
                  );
          return ret;

}

SYSCALL_PROTO(3)(uint64_t n, uint64_t arg1, uint64_t arg2,uint64_t arg3) {
	uint64_t __res;
	__asm__ volatile ("int $0x80" \
	                  : "=a" (__res) \
			    : "0" (n) ,"b"((uint64_t)(arg1)),"c"((uint64_t)(arg2)),"d"((uint64_t)(arg3)));
	return __res;
}



/******* SYSTEM CALLS ********/
void sys_putchar(uint64_t c);
void sys_scanf(char* buffer,uint64_t size);
int sys_get_pid();
int sys_get_ppid();
int sys_fork();
int sys_execve(char *filename, char *argv[], char *en[]);
int sys_exit();
int sys_list_process();
int sys_read(uint64_t n,uint64_t addr, uint64_t len);
void sys_write(uint64_t n,uint64_t addr, uint64_t len);
void sys_sleep(int seconds);
int ls();


enum syscall_num {
	PRINTF,
	SCANF,
	MALLOC,
	BRK,
	FORK,
	EXECVE,
	WAIT,
	WAITPID,
	EXIT,
	MMAP,
	MUNMAP,
	GETPID,
	GETPPID,
	LISTPROCESS,
	UPDATE_ENV,
	UPDATEPATH,
	CURRENTPATH,
	OPENDIR,
	READDIR,
	CLOSEDIR,
	OPEN,
	READ,
	WRITE,
	CLOSE,
	SLEEP,
	CLEAR,
	YIELD,
	DISABLE_INTERRUPT,
	ENABLE_INTERRUPT,
	NUM_SYSCALLS
};

/*
#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
	long __res; \
	__asm__ volatile (  "int $0x80" \
       	           : "=a" (__res) \
              	    : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
                    	"d" ((long)(arg3))); \
	__syscall_return(type,__res); \
}

SYSCALL_PROTO(0)(uint64_t n) {
	return 0;
}

SYSCALL_PROTO(1)(uint64_t n, uint64_t a1) {
	return 0;
}

SYSCALL_PROTO(2)(uint64_t n, uint64_t a1, uint64_t a2) {
	return 0;
}

SYSCALL_PROTO(3)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	return 0;
}

SYSCALL_PROTO(4)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
	return 0;
}
*/


#endif
