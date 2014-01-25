#ifndef _SCHED_H
#define _SCHED_H

#include <defs.h>


#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE      1
#define TASK_UNINTERRUPTIBLE    2
#define TASK_STOPPED            4
#define TASK_TRACED             8
/* in tsk->exit_state */
#define EXIT_ZOMBIE             16
#define EXIT_DEAD               32
/* in tsk->state again */
#define TASK_NONINTERACTIVE     64
#define TASK_WAITING			    128

#define STACK_SIZE 1024

extern int global_pid; //0 is reserved for bash shell

struct task_struct *current_task;
struct task_struct *head;

struct task_struct {
	struct mm_struct *mm;
	uint64_t rip;
	uint64_t rsp;
       uint64_t stack[STACK_SIZE];
       void (*fptr)(void);
	uint64_t ppid;
	uint64_t pid;
	volatile long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
	long counter;
	long priority;
	struct task_struct *next_task;
	struct task_struct *parent;
	struct task_struct *child;
	uint64_t cr3;
	uint64_t ustack;	//for base address of user stack
	uint64_t uheap;	//for base address of user heap
	uint64_t cur_stack;  //for current stack pointer
	uint64_t cur_heap;	//for current heap pointer..will increase when sbrk() is called
	int sleep_time;
	char *name;
};

struct mm_struct {
        struct vm_area_struct  *mmap;               /* list of memory areas */
        //struct rb_root         mm_rb;               /* red-black tree of VMAs */
        struct vm_area_struct  *mmap_cache;         /* last used memory area */
        unsigned long          free_area_cache;     /* 1st address space hole */
        //pgd_t                  *pgd;                /* page global directory */
        //atomic_t               mm_users;            /* address space users */
        int                    mm_count;            /* primary usage counter */
        int                    map_count;           /* number of memory areas */
        //struct list_head       mmlist;              /* list of all mm_structs */
        unsigned long          start_code;          /* start address of code */
        unsigned long          end_code;            /* final address of code */
        unsigned long          start_data;          /* start address of data */
        unsigned long          end_data;            /* final address of data */
        unsigned long          rss;                 /* pages allocated */        
	 int pid;
};

struct taskList {
	struct task_struct *task;
	struct taskList *next;
};

struct task_struct *getCurrentTask(struct taskList *list);
struct taskList *addToHeadTaskList(struct taskList *list, struct task_struct *task_struct);
struct taskList *addToTailTaskList(struct taskList *list, struct task_struct *task_struct);
struct taskList *removeFromTaskList(struct taskList *list, struct task_struct *task_struct);
struct taskList *moveTaskToEndOfList(struct taskList *list);

extern struct taskList *allProcesses;
extern struct taskList *runQueue;
extern struct taskList *waitQueue;
extern struct taskList *deadQueue;

void setup_processes();
void schedule();
void init_kernel_process(struct task_struct * proc, void * func);
void WaitSysCall();
void WaitPidSysCall(int waitId);

int FindFullPath(char *fullPath);

#define for_each_task(p) \
	for (p = &init_task ; (p = p->next_task) != &init_task ; )

#define next_task(p)    list_entry((p)->tasks.next, struct task_struct, tasks)


#endif