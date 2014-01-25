#include <stdio.h>
#include <sys/sched.h>
#include <sys/pmap.h>
#include <sys/pagetab.h>
#include <sys/mm.h>
#include <sys/elf.h>
#include <sys/gdt.h>
#include <math.h>
#include <stdlib.h>
#include <sys/idt.h>

struct task_struct *task1, *task2, *task3;

struct task_struct *current_task = NULL;

uint64_t rip1 = 0;

uint64_t *stack;

uint64_t curStack;

uint64_t userStackAddr = 0x50000;

int is_first_switch = 0;

uint64_t cur_rsp = 0;

uint64_t cur_top = 0;

uint64_t parent_rsp;

struct taskList *allProcesses;
struct taskList *runQueue;
struct taskList *waitQueue;
struct taskList *deadQueue;

extern struct isr_stack_frame* g_regVals1;

int global_pid = 1;

void idle_process()
{
	while(1)
	{
		printf("Inside idle process");
	}
}

struct task_struct *getCurrentTask(struct taskList *list)
{
	if (list == NULL)
	{
		//printf("no list found");
		return NULL;
	}

	return list->task;
}

struct taskList *addToHeadTaskList(struct taskList *list, struct task_struct *new_task_struct)
{
	if (new_task_struct == NULL)
	{
		//printf("task is null");
		return NULL;
	}

	if (list == NULL)
	{
		list = (struct taskList *)kmalloc(sizeof(struct taskList));
		list->task = new_task_struct;
		list->next = NULL;
		return list;
	}

	struct taskList *temp = (struct taskList *)kmalloc(sizeof(struct taskList));
	temp->task = new_task_struct;
	temp->next = list;
	list = temp;
	return list;
}

struct taskList *addToTailTaskList(struct taskList *list, struct task_struct *new_task_struct)
{
	if (new_task_struct == NULL)
	{
		//printf("task is null");
		return list;
	}

	if (list == NULL)
	{
		list = (struct taskList *)kmalloc(sizeof(struct taskList));
		list->task = new_task_struct;
		list->next = NULL;
		return list;
	}

	struct taskList *start = list;

	while (start->next != NULL)
		start = start->next;

	
	struct taskList *temp = (struct taskList *)kmalloc(sizeof(struct taskList));
	temp->task = new_task_struct;
	temp->next = NULL;
	start->next = temp;

	return list;
}

struct taskList *removeFromTaskList(struct taskList *list, struct task_struct *task_struct)
{
	struct taskList *start = list;

	if (list == NULL || task_struct == NULL) 
	{
		//printf("Fault.. No tasks exists.. !!\n");
		return NULL;
	}

	if (list->task == task_struct) 
	{
		list = list->next;
		return list;
	}

	if (list->next != NULL && list->next->task != task_struct)
		list = list->next;

	//while (list->next->task != task_struct)
	//	list = list->next;

	if (list->next == NULL)
	{
		//printf("task not found");
		return start;
	}

	if (list->next->next != NULL)
		list->next = list->next->next;
	else
		list->next = NULL;

	return start;
}

struct taskList *moveTaskToEndOfList(struct taskList *list)
{
	if(list == NULL)
	{
		return list;
	}
	if (list->next == NULL)
	{
		return list;
	}	
	struct taskList *start = list;
	struct taskList *temp = list;
	list = list->next;
	while (temp->next)
		temp = temp->next;
	temp->next = start;
	start->next = NULL;
	return list;
}

int sys_fork()
{
	/*asm volatile(
			"movq %%rsp, %0"
			:"=m"(parent_rsp)
			:
			:"memory"
	);*/

	struct task_struct *new_task = alloc_new_task();

	// set current task to be the "parent" of the new task
	new_task->parent = current_task;
	new_task->name = "CHILD";

	// set new PID
	new_task->pid = ++global_pid;

	// get the cr3 of the current task so as to re-load it at the end
	struct PML4 * current_task_cr3 = (struct PML4 *)current_task->cr3;

	copy_page_tables((uint64_t)current_task_cr3, (uint64_t)new_task->cr3);

	setCR3((struct PML4 *)new_task->cr3);
	
	copy_task_struct(current_task, new_task);

	
	//LoadDataInMemory1(new_task);


	//Assign VMA to STACK SO THAT WE CAN REMOVE IT AT THE TIME OF exit system call

	//new_task->rsp =  (uint64_t)kmalloc(PAGE_SIZE);	 //allocate a blank page in kernel for stack

	/*struct vm_area_struct *vma_stack = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));   //for stack allocation
	vma_stack->vm_mm = new_task->mm;
	vma_stack->vm_start = new_task->rsp - PAGE_SIZE ;
	vma_stack->vm_end = new_task->rsp ;
	vma_stack->vm_flags = (PF_R + PF_W);
	vma_stack->vm_next = NULL;

	if(new_task->mm->mmap == NULL)
		new_task->mm->mmap = vma_stack;
	else {
		struct vm_area_struct *current_vma = new_task->mm->mmap;
		while(current_vma->vm_next != NULL)
			current_vma = current_vma->vm_next;
		current_vma->vm_next = vma_stack;
	}*/


	// ENQUEUE PROCESS IN THE TASK LIST


	//asm volatile( "movq %0, %%rsp ": : "m"(new_task->rsp) : "memory" );


	new_task->state = TASK_RUNNING;


	__asm volatile("movq %%rsp, %0;":"=g"(curStack));


	uint64_t strTop = (uint64_t)&(new_task->stack[1023]);

	asm volatile( "movq %0, %%rsp ": : "m"(strTop) : "memory" );

	asm volatile("pushq $0x23\n\t"
				"pushq %0\n\t"
				"pushq $0x200\n\t"
				"pushq $0x1b\n\t"
				"pushq %1\n\t"
				: :"m"(g_regVals1->rsp),"m"(g_regVals1->rip) :"memory");

	__asm volatile("pushq %0\n\t"
				"pushq $0x0\n\t"
				"pushq %2\n\t"
				"pushq %3\n\t"
				"pushq %4\n\t"
				"pushq %5\n\t"
				"pushq %6\n\t"
				"pushq %7\n\t"
				"pushq %8\n\t"
				"pushq %9\n\t"
				"pushq %10\n\t"
				"pushq %11\n\t"
				"pushq %12\n\t"
				"pushq %13\n\t"
				"pushq %14\n\t"
				: :"m"(g_regVals1->rbp),"m"(g_regVals1->rax),"m"(g_regVals1->rbx),"m"(g_regVals1->rcx),"m"(g_regVals1->rdx),"m"(g_regVals1->rsi),"m"(g_regVals1->rdi),"m"(g_regVals1->r8),"m"(g_regVals1->r9),"m"(g_regVals1->r10),"m"(g_regVals1->r11),"m"(g_regVals1->r12),"m"(g_regVals1->r13),"m"(g_regVals1->r14),"m"(g_regVals1->r15):"memory");
	
	g_regVals1->rax =  new_task->pid;
	new_task->rsp = current_task->rsp;

	/*int i=0;
	for(i=0;i<64;i++)
		new_task->stack[i] = current_task->stack[i];
	 int ps;
	*/

	__asm volatile("movq %%rsp, %0;":"=g"(new_task->rsp));

	__asm volatile( "movq %0, %%rsp ": : "m"(curStack) : "memory" );

		setCR3(current_task_cr3);


	allProcesses = addToTailTaskList(allProcesses, new_task);
	//schedule();
	return 0;
}

void schedule()
{
	while(allProcesses!=NULL &&allProcesses->next != NULL)
	{
		current_task = allProcesses->task;
		allProcesses = moveTaskToEndOfList(allProcesses);
		if (current_task->sleep_time > 0)
			continue;
		else
			break;
	}

	if (current_task)
	{
		if (!is_first_switch)
		{
			is_first_switch = 1;			

			asm volatile("movq %0, %%cr3":: "a"(current_task ->cr3));

			asm volatile(
				"movq %0, %%rsp "
				:
				:"m"(current_task->rsp)
				:"memory"
			);

			tss.rsp0 = (uint64_t)&current_task->stack[1023];

			asm volatile("mov $0x28,%ax");

			asm volatile("ltr %ax");

			__asm volatile(
				"popq %%r15\n\t"
				"popq %%r14\n\t"
				"popq %%r13\n\t"
				"popq %%r12\n\t"
				"popq %%r11\n\t"
				"popq %%r10\n\t"
				"popq %%r9\n\t"
				"popq %%r8\n\t"
				"popq %%rdi\n\t"
				"popq %%rsi\n\t"
				"popq %%rdx\n\t"
				"popq %%rcx\n\t"
				"popq %%rbx\n\t"
				"popq %%rax\n\t"
				"popq %%rbp\n\t"
				: : :"memory"
			);

			asm volatile("iretq");
		}
		else
		{
			/*asm volatile(
				"movq %%rsp, %0"
				:"=m"(current_task->rsp)
				:
				:"memory"
			);*/

			/*struct task_struct * temp = current_task;
			while(temp->next_task != NULL)
			{
				temp = temp->next_task;
			}
			temp->next_task = current_task;
			temp = temp->next_task;
			current_task = current_task->next_task;
			temp->next_task = NULL;*/

			asm volatile("movq %0, %%cr3":: "a"(current_task ->cr3));

			asm volatile(
				"movq %0, %%rsp "
				:
				:"m"(current_task->rsp)
				:"memory"
			);

			tss.rsp0 = (uint64_t)&current_task->stack[1023];

			__asm volatile(
				"popq %%r15\n\t"
				"popq %%r14\n\t"
				"popq %%r13\n\t"
				"popq %%r12\n\t"
				"popq %%r11\n\t"
				"popq %%r10\n\t"
				"popq %%r9\n\t"
				"popq %%r8\n\t"
				"popq %%rdi\n\t"
				"popq %%rsi\n\t"
				"popq %%rdx\n\t"
				"popq %%rcx\n\t"
				"popq %%rbx\n\t"
				"popq %%rax\n\t"
				"popq %%rbp\n\t"
				: : :"memory"
			);

			asm volatile("iretq");
		}
	}
}

void func1()
{
	while(1)
	{
		printf("\nHello");
		//__asm__("int $0x80");
		//schedule();
	}
}

void func2()
{
	while(1)
	{
		printf("\nWorld");
		//__asm__("int $0x80");
		//schedule();
	}
}

void func3()
{
	while(1)
	{
		printf("\nBye");
		//__asm__("int $0x80");
		//schedule();
	}
}



void WaitSysCall()
{
	struct taskList * temp = deadQueue;

	if (temp == NULL)
		printf("There are no processes in the Dead queue");

	while(temp->next)
	{
		if(temp->task->ppid == current_task->pid)
		{
			current_task->state = TASK_WAITING;
			waitQueue = addToTailTaskList(waitQueue,current_task);
			allProcesses = removeFromTaskList(allProcesses, current_task);
			schedule();
		}
		temp = temp->next;
	}	
}

void WaitPidSysCall(int waitId)
{

	//int waitId = (int)g_regVals->rbx;
	//printf("waitId %d\n",waitId);

//	struct task_struct *parent = getCurrentTask(allProcesses);
	struct task_struct *parent = current_task;
	struct taskList *list = allProcesses;
	struct task_struct *curr = NULL;
	while(list != NULL && list->task != NULL)
	{
		curr = list->task;
		if(waitId == curr->pid)
		{
			parent->state = TASK_WAITING;
			waitQueue = addToTailTaskList(waitQueue, parent);
			allProcesses = removeFromTaskList(allProcesses, parent);
			curr = allProcesses->next->task;
			break;
		}
		else
		{
			list=list->next;
		}
	}

	schedule();
}



void init_process(struct task_struct * proc, void * func)
{

	uint64_t cur_top = (uint64_t)&proc->stack[1023];

	uint64_t user_stack = (uint64_t)kmalloc(PAGE_SIZE)+ PAGE_SIZE-16; // 2*PAGE_SIZE is working it seems

	__asm volatile("movq %%rsp, %0;":"=g"(cur_rsp));

	__asm volatile( "movq %0, %%rsp ": : "m"(cur_top) : "memory" );


	__asm volatile(
		"pushq $0x23\n\t"
		"pushq %0\n\t"
		"pushq $0x200\n\t"
		"pushq $0x1b\n\t"
		"pushq %1\n\t"
		: :"c"((uint64_t)(user_stack)),"d"((uint64_t)func) :"memory"
	);


	__asm volatile(
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"				
		"pushq $0x0\n\t"
		: : :"memory"
	);


	proc->rip = (uint64_t)func;

	proc->cr3 =  getNewPML4ForUserProcess();

	__asm volatile("movq %%rsp, %0;":"=g"(proc->rsp));

	__asm volatile( "movq %0, %%rsp ": : "m"(cur_rsp) : "memory" );

}

void add_proc_to_task_list(void * func)
{
	struct task_struct * task = kmalloc(sizeof(struct task_struct));
	init_process(task, func);

	if (current_task == NULL)
	{
		current_task = task;
		current_task->next_task = NULL;
	}
	else
	{
		struct task_struct * temp = current_task;
		while(temp->next_task != NULL)
		{
			temp = temp->next_task;
		}
		temp->next_task = task;
		task->next_task = NULL;
	}
}

struct task_struct * get_user_proc(char filename[])
{
	uint64_t cr3 = getNewPML4ForUserProcess();

	uint64_t curr_cr3 = getCR3();
	asm volatile ("movq %0, %%cr3":: "a"(cr3));

	struct task_struct * task = load_elf(filename);
	task->cr3 = cr3;

	task->pid = global_pid++;



	LoadDataInMemory1(task);

	asm volatile("movq %0, %%cr3":: "a"(curr_cr3));

	__asm volatile("movq %%rsp, %0;":"=g"(cur_rsp));

	cur_top = (uint64_t)&task->stack[1023];

	__asm volatile( "movq %0, %%rsp ": : "m"(cur_top) : "memory" );


	__asm volatile(
		"pushq $0x23\n\t"
		"pushq %0\n\t"
		"pushq $0x200\n\t"
		"pushq $0x1b\n\t"
		"pushq %1\n\t"
		: :"c"((uint64_t)(task->cur_stack)),"d"((uint64_t)task->rip) :"memory"
	);


	__asm volatile(
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"				
		"pushq $0x0\n\t"
		: : :"memory"
	);


	asm volatile("movq %%rsp, %0;":"=g"(task->rsp));

	asm volatile( "movq %0, %%rsp ": : "m"(cur_rsp) : "memory" );


	task->pid = ++global_pid;
	task->state = TASK_RUNNING;

	return task;
}

int sys_execve(char *filename, char *argv[], char *en[])
{
	int isElf = checkforElf(filename);
	if (isElf == -1)
		return -1;

	free_task_struct(current_task);
	empty_page_tables((uint64_t) current_task->cr3);

	load_elf_into_task(filename, current_task);

	// map vma onto new page tables
	LoadDataInMemory1(current_task);


	__asm volatile("movq %%rsp, %0;":"=g"(cur_rsp));

	cur_top = (uint64_t)&current_task->stack[1023];

	__asm volatile( "movq %0, %%rsp ": : "m"(cur_top) : "memory" );


	__asm volatile(
		"pushq $0x23\n\t"
		"pushq %0\n\t"
		"pushq $0x200\n\t"
		"pushq $0x1b\n\t"
		"pushq %1\n\t"
		: :"c"((uint64_t)(current_task->cur_stack)),"d"((uint64_t)current_task->rip) :"memory"
	);


	__asm volatile(
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"				
		"pushq $0x0\n\t"
		: : :"memory"
	);

	asm volatile("movq %%rsp, %0;":"=g"(current_task->rsp));

	asm volatile("movq %0, %%rsp ": : "m"(cur_rsp) : "memory" );

	current_task->state = TASK_RUNNING;

	//allProcesses = addToHeadTaskList(allProcesses, current_task);

	schedule();

	return 0;
}

void sys_sleep(int seconds)
{
	if (seconds > 0)
	{
		current_task->sleep_time = seconds;
		allProcesses = removeFromTaskList(allProcesses, current_task);
		waitQueue = addToTailTaskList(waitQueue, current_task);
		schedule();
	}
}

void setUserProcess(char fileName[100])
{
	struct task_struct * tsk = load_elf("bin/hello");
	LoadDataInMemory1(tsk);

	tsk->pid = 1000;
	tsk->cr3 =  getNewPML4ForUserProcess();
	
	if (current_task == NULL)
	{
		current_task = tsk;
		current_task->next_task = NULL;
	}
	else
	{
		struct task_struct * temp = current_task;
		while(temp->next_task != NULL)
		{
			temp = temp->next_task;
		}
		temp->next_task = tsk;
		tsk->next_task = NULL;
	}
}

uint64_t getpid()
{
	return current_task->pid;
}

uint64_t getppid()
{
	return current_task->ppid;
}

int sys_list_process()
{
	int count = 0;

	printf("\nPROCESS NAME            PID\n");
	printf("-----------------------------\n\n");

    	printf("Running Processes---------------------------------------\n");

	struct taskList *start = allProcesses;
	while (start != NULL && start->task != NULL) {
		printf("%s\t", start->task->name);
		printf("\t    %d\n", start->task->pid);
		start = start->next;
	}

	printf("\nWaiting Processes---------------------------------------\n");

	start = waitQueue;
	while (start != NULL && start->task != NULL) {
		printf("%s\t", start->task->name);
		printf("\t    %d\n", start->task->pid);
		start = start->next;
	}

	printf("\nDead Processes------------------------------------------\n");

	start = deadQueue;
	while (start != NULL && start->task != NULL) {
		printf("%s\t", start->task->name);
		printf("\t    %d\n", start->task->pid);
		start = start->next;
	}

	return count;
}


void test ()
{
	/*
	struct task_struct *bash_task = NULL;
	char *name = "bin/bash";

	// read elf from tarfs
	// create code vma, stack vma and heap vma
	bash_task = load_elf(filename);

	bash_task->pid = 0;
	bash_task->cr3 = getNewPML4ForUserProcess();

	asm volatile ("movq %0, %%cr3":: "a"(bash_task->cr3));

	// map vma onto new page tables
	LoadDataInMemory1(bash_task);

	current_task = bash_task;

	__asm volatile("movq %%rsp, %0;":"=g"(cur_rsp));

	cur_top = (uint64_t)&task->stack[1023];

	__asm volatile( "movq %0, %%rsp ": : "m"(cur_top) : "memory" );


	__asm volatile(
		"pushq $0x23\n\t"
		"pushq %0\n\t"
		"pushq $0x200\n\t"
		"pushq $0x1b\n\t"
		"pushq %1\n\t"
		: :"c"((uint64_t)(task->ustack)),"d"((uint64_t)task->rip) :"memory"
	);


	__asm volatile(
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"
		"pushq $0x0\n\t"				
		"pushq $0x0\n\t"
		: : :"memory"
	);

	asm volatile("movq %%rsp, %0;":"=g"(task->rsp));

	asm volatile( "movq %0, %%rsp ": : "m"(cur_rsp) : "memory" );

	task->state = TASK_RUNNING;

	addToHeadTaskList(runQueue, task);

	*/
}

void setup_processes()
{

	struct task_struct *idle_task = get_user_proc("bin/idle");
	idle_task->name = "IDLE";
	idle_task->sleep_time = 0;
	allProcesses = addToTailTaskList(allProcesses, idle_task);

	//struct task_struct *fork_task = get_user_proc("bin/fork");
	//fork_task->name = "FORK";
	//fork_task->sleep_time = 0;
	//allProcesses = addToTailTaskList(allProcesses, fork_task);

	//struct task_struct *opendir_task = get_user_proc("bin/opendir");
	//allProcesses = addToTailTaskList(allProcesses, opendir_task);


	struct task_struct *shell_task = get_user_proc("bin/shell");
	shell_task->name = "SHELL";
	shell_task->sleep_time = 0;
	allProcesses = addToTailTaskList(allProcesses, shell_task);
	
	//struct task_struct *ls_task = get_user_proc("bin/ls");
	//allProcesses = addToTailTaskList(allProcesses, ls_task);


	//struct task_struct *scanf_task = get_user_proc("bin/scanf");
	//scanf_task->name = "SCANF";	
	//scanf_task->sleep_time = 0;
	//allProcesses = addToTailTaskList(allProcesses, scanf_task);
}
