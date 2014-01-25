#include <defs.h>
#include <stdio.h>
#include <sys/idt.h>
#include <sys/elf.h>
#include <sys/sched.h>
#include <sys/pmap.h>
#include <sys/pagetab.h>

extern void _isr0();
extern void _isr1();
extern void _isr13();
extern void _isr14();
extern void _isr128();
extern void _isr129();

void init_isr()
{
	idt_set_gate(0, 8, 0, 0x0e, 0, ((uint64_t)&_isr0));
    	//idt_set_gate(1, 8, 0, 0x0e, 0, ((uint64_t)&_isr1)); // we can use this handler later if we want
	idt_set_gate(13, 8, 0, 0x0e, 0, ((uint64_t)&_isr13));
	idt_set_gate(14, 8, 0, 0x0e, 0, ((uint64_t)&_isr14));
	idt_set_gate(128, 8, 0, 0x0e, 3, ((uint64_t)&_isr128));
	idt_set_gate(129, 8, 0, 0x0e, 3, ((uint64_t)&_isr129));
}

char *exception_messages[] =
{
    "Division By Zero"
};

//r->int_no is always 16 for the dvide by zero no matter what u push in the stack
void fault_handler()
{
	//printf("a constant value in regs %d", r->int_no);
	__asm__("hlt");
}

void interrupt_general_protection_fault()
{
	printf("General protection fault occurred!");
	__asm__( "hlt" );
}

void interrupt_page_fault(isr_stack_frame_t isr_stack)
{

	//printf("\nPage fault occurred!");

	//printf("\nMapping the faulting address");

	uint64_t faultAddr;
    	__asm volatile("mov %%cr2, %0" : "=r" (faultAddr));
	uint64_t COW  = 0x10000000000000UL;
	uint64_t PTentry = getMappedPhysicalAddr(faultAddr);

	//printf("\nPage Fault accessing : %p",faultAddr);

	if((COW&PTentry)  == COW)
	{

		CopyVmaData(faultAddr);

	}
	/*else
	{
		CopyCodeData(faultAddr);
	}
	else
	{
		kmalloc_virtual(PAGE_SIZE, faultAddr);
	}*/

	//printf("\nSuccess");

	//__asm__( "hlt" );
}

void interrrupt_schedule()
{
	//schedule();
}
