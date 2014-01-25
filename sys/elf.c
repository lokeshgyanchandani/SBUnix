#include <defs.h>
#include <stdio.h>
#include <math.h>
#include <sys/tarfs.h>
#include <sys/elf.h>
#include <sys/pmap.h>
#include <sys/mm.h>
#include <sys/sched.h>
#include <sys/pagetab.h>
#include <sys/pmap.h>

#define COW  0x10000000000000UL
#define VIRT_OFFSET 0xFFFFFFFF80000000

uint64_t file_start = 0;
uint64_t vm_pgoff = 0;
uint64_t vm_sz = 0;


void* readTarfile(char *file_name) {

    struct posix_header_ustar * header = (struct posix_header_ustar*)(&_binary_tarfs_start);
    int size_of_file = 0; 
    
    do {
		/*convert header->size string to octal to integer*/
		size_of_file = octal_decimal(atoi(header->size));
		if(strcmp(file_name, header->name) == 0)
		{
			//printf("File Name: %s..File Size: %d bytes..Contents of file: ", header->name, size_of_file);
			return (void *)header;
		}

		if(size_of_file > 0) 
			header = header + 1 + (size_of_file / 513 + 1);
       	else
       		header = header + 1;
	} while(header < (struct posix_header_ustar*)&_binary_tarfs_end);

	return NULL;    
}

void populateTask(elf64_eheader *elf_ehdr, elf64_pheader *elf_phdr,struct task_struct *tsk)
{
	if(tsk->mm == NULL)
		return;

	struct vm_area_struct *vma = populate_vma(elf_ehdr, elf_phdr);
    	
	if(tsk->mm->mmap == NULL)
		tsk->mm->mmap = vma;        
	else {
		struct vm_area_struct *curr = tsk->mm->mmap;
		while(curr->vm_next != NULL)
			curr = curr->vm_next;
		curr->vm_next = vma;       
	}

	vma->vm_mm = tsk->mm;

};

struct task_struct* load_elf(char *filename)
{
	return load_elf_into_task(filename, NULL);
}

int checkforElf(char *filename)
{
	struct posix_header_ustar *elf_header = (struct posix_header_ustar *)readTarfile(filename);

	if(elf_header == NULL)
		return -1;
	else
		return 0;
}

struct task_struct* load_elf_into_task(char *filename, struct task_struct * tsk)
{
	struct elf64_eheader *elf_ehdr;
	struct elf64_pheader *elf_phdr;

	struct posix_header_ustar *elf_header = (struct posix_header_ustar *)readTarfile(filename);

	if(elf_header == NULL)
		return NULL;

	elf_ehdr = (struct elf64_eheader *)(elf_header+1);
	elf_phdr = (struct elf64_pheader *)((uint64_t)elf_ehdr + elf_ehdr->e_phoff);

	if (tsk == NULL)
	{
		tsk = kmalloc(sizeof(struct task_struct));
	}
	tsk->mm = kmalloc(sizeof(struct mm_struct));

	tsk->rip = elf_ehdr->e_entry;   //entry point address of .text section	


	file_start = (uint64_t)elf_ehdr;
	vm_pgoff = elf_phdr->p_offset;
	vm_sz = elf_phdr->p_filesz;

	tsk->mm->mmap = NULL;            

	elf_phdr = (struct elf64_pheader *)((uint64_t)elf_ehdr + elf_ehdr->e_phoff);
	int i=0;

	for (i = 0; i < elf_ehdr->e_phnum; ++i) {
		if(elf_phdr->p_type == PT_LOAD) {
			populateTask(elf_ehdr, elf_phdr, tsk);
		}
		elf_phdr++;
	}

	tsk->ustack = _UHEAP;
	tsk->uheap = _UHEAP;

	tsk->cur_heap = tsk->uheap;

	uint64_t uustack = (uint64_t)sbrk(PAGE_SIZE) + PAGE_SIZE;
	struct vm_area_struct *vma_stack = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));   //for stack allocation
	vma_stack->vm_mm = tsk->mm;
	vma_stack->vm_start = uustack- PAGE_SIZE;
	vma_stack->vm_end = uustack;
	tsk->cur_stack = _UHEAP+PAGE_SIZE - 16;
	vma_stack->vm_flags = (PF_R + PF_W);
	vma_stack->vm_next = NULL;
	vma_stack->isStack = 1;

	if(tsk->mm->mmap == NULL)
		tsk->mm->mmap = vma_stack;        
	else {
		struct vm_area_struct *curr = tsk->mm->mmap;
		while(curr->vm_next != NULL)
			curr = curr->vm_next;
		curr->vm_next = vma_stack;       
	}


	//IN THE SAME FASHION ASSIGN HEAP TO VMA WHICH WE CAN CLEANUP AT THE TIME OF exit system call
	//either map this area right now and back it with physical page or make sure that page fault handler is working
	struct vm_area_struct *vma_heap = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));   //for stack allocation
	vma_heap->vm_mm = tsk->mm;
	vma_heap->vm_start = (uint64_t)kmalloc(PAGE_SIZE);
	vma_heap->vm_end = vma_heap->vm_start+PAGE_SIZE;
	vma_heap->vm_next = NULL;
	vma_heap->isHeap = 1;

	if(tsk->mm->mmap == NULL)
		tsk->mm->mmap = vma_heap;        
	else {
		struct vm_area_struct *curr = tsk->mm->mmap;
		while(curr->vm_next != NULL)
			curr = curr->vm_next;
		curr->vm_next = vma_heap;       
	}

	return tsk;
}

//with vma virtual address
int LoadDataInMemory(struct task_struct* tsk,uint64_t virtAddr)
{
	int success = 0;
	struct vm_area_struct* curVma = tsk->mm->mmap;

	while(curVma != NULL)
	{
		if(curVma->vm_start <= virtAddr && virtAddr <= curVma->vm_end)
		{
			
			kmalloc_virtual(curVma->vm_end-curVma->vm_start,curVma->vm_start);

			int totSize = curVma->vm_end - curVma->vm_start;

			char* phyStart = (char*)(curVma->vm_file_file_start+ curVma->vm_file_vm_pgoff);
			char* addr = (char*)curVma->vm_start;
			while(totSize--)
    				*addr ++ = *phyStart++;

			success = 1;
			break;			
		}
		curVma = curVma->vm_next;

	}
	return success;
}

//without virtual address
int LoadDataInMemory1(struct task_struct* tsk)
{
	int success = 0;
	struct vm_area_struct* curVma = tsk->mm->mmap;

	while(curVma != NULL)
	{
		if(curVma->isStack != 1 && curVma->isHeap != 1)
		{

			kmalloc_virtual(curVma->vm_end-curVma->vm_start,curVma->vm_start);

			int totSize = curVma->vm_end - curVma->vm_start;

			char* phyStart = (char*)(curVma->vm_file_file_start+ curVma->vm_file_vm_pgoff);
			char* addr = (char*)curVma->vm_start;
			while(totSize--)
    				*addr ++ = *phyStart++;

			success = 1;
			break;			
		}
		curVma = curVma->vm_next;

	}
	return success;
}

//page fault copier
int CopyVmaData(uint64_t virtAddr)
{


       uint64_t base = VIRT_OFFSET;

	struct mm_struct *memStr = (struct mm_struct *)current_task->mm;
	int success = 0;
	if(memStr != NULL)
	{
		struct vm_area_struct* curVma = memStr->mmap;

		while(curVma != NULL)
		{
			if(curVma->vm_start <= virtAddr && virtAddr <= curVma->vm_end)
			{
				uint64_t PTentry = getMappedPhysicalAddr(curVma->vm_start);

				//Create a new Physical Page fr this VMA into which we will copy the contents from the old physical address it was pointing at
				kmalloc_virtual(PAGE_SIZE,curVma->vm_start);
				mmap((uint64_t)(base|(PTentry&~COW)),(uint64_t)(PTentry&~COW));
				memcpy((void *)(base|(PTentry&~COW)),(void *)(curVma->vm_start),4096);

				success = 1;
				break;			
			}
      curVma = curVma->vm_next;
		}
	}
	return success;
}

