#include <defs.h>
#include <stdio.h>
#include <sys/io.h>
#include <sys/pagetab.h>
#include <sys/pmap.h>
#include <sys/mm.h>
#include <math.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/timer.h>
#include <sys/tarfs.h>


void* kernelbase;
void* kernelfree;

void start(uint32_t* modulep, void* physbase, void* physfree)
{
	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;

	while(modulep[0] != 0x9001) modulep += modulep[1]+2;	

	init_freelist(physbase, physfree);
	physfree = kernelfree;
	clrscr();

	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
			printf("Available Physical Memory [%x-%x]\n\n", smap->base, smap->base + smap->length);
			set_kernel_in_freelist(smap->base, smap->base + smap->length);
		}
	}

	// kernel starts here

	uint64_t *pml4 = setup_page_tables((uint64_t)physbase, (uint64_t)physfree);
	
	printf("pml4 phys addr: %x\n", pml4);

	//handle_page_fault(pml4, 0xFFFFFF7FBFDFE000);
	//handle_page_fault(pml4);

	//asm volatile("cli");

	set_timer_phase();
	
	setup_processes();


	//asm volatile("sti");

	clrscr();
/* ################################ TARFS ################################################# */
/*
//loading of tarfs files
printf(" start and end address %x %x",&_binary_tarfs_start,&_binary_tarfs_end);
//struct posix_header_ustar* header =  (struct posix_header_ustar*)get_elf_file(&_binary_tarfs_start);
//print_posix_header(header);
// done loading of tarfs files

tarfs_table_initialise(&_binary_tarfs_start);
print_file_table(tarfs_table);
*/
/* ################################################################################# */

	//readTarfile("bin/hello");

	//struct task_struct * elf_struct = load_elf("bin/hello");
	//printf("elf_structure:: %x", elf_struct);


	//printf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	__asm__(
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE])
	);
	reload_gdt();
	reload_idt();
	setup_tss();
	start(
		(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
		&physbase,
		(void*)(uint64_t)loader_stack[4]
	);
	while(1);
}