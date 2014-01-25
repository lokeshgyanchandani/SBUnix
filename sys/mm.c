#include <defs.h>
#include <stdio.h>
#include <sys/pmap.h>
#include <sys/mm.h>
#include <sys/pagetab.h>
#include <sys/elf.h>
#include <sys/sched.h>
#include <stdio.h>
#include <sys/tarfs.h>

extern char kernmem;

extern uint64_t parent_rsp;

//void open(struct vm_area_struct *area);
//void close(struct vm_area_struct *area);

struct vm_area_struct *find_vma(struct mm_struct *mm, unsigned long addr)
{
    if (mm != NULL) {
        struct vm_area_struct * curr;
        curr = mm->mmap;

        while (curr != NULL) {
	        if (curr && curr->vm_end > addr && curr->vm_start <= addr)
                    return curr;
               curr = curr->vm_next;
		
        }
    }
    return NULL;
}

struct vm_area_struct* populate_vma(elf64_eheader *elf_ehdr, elf64_pheader *elf_phdr)
{
    struct vm_area_struct *vma = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct)); 

    vma->vm_start = elf_phdr->p_vaddr;
    vma->vm_end = elf_phdr->p_vaddr + elf_phdr->p_memsz;
    vma->vm_flags = elf_phdr->p_flags;
    vma->vm_pgoff = elf_phdr->p_offset;
    vma->vm_file_file_start = (uint64_t)elf_ehdr;
    vma->vm_file_vm_pgoff = elf_phdr->p_offset;
    vma->vm_file_vm_sz = elf_phdr->p_filesz;
    vma->vm_next = NULL;
    vma->isHeap = 0;
    vma->isStack = 0;
    return vma;

};

struct task_struct * alloc_new_task()
{
	struct task_struct *new_proc;
	struct mm_struct * mm_s;

	mm_s = kmalloc(sizeof(struct mm_struct));
	mm_s->mmap = NULL;

	new_proc = kmalloc(sizeof(struct task_struct));
	new_proc->mm = mm_s;
	new_proc->cr3 = getNewPML4ForUserProcess();
	new_proc->next_task = NULL;
	new_proc->child = NULL;
	new_proc->parent = NULL;
	new_proc->ppid = 0;

	new_proc->ustack = _USTACK;
	new_proc->uheap = _UHEAP;

	new_proc->cur_heap = new_proc->uheap;
	new_proc->cur_stack = new_proc->ustack;

	return new_proc;	
}

void copy_task_struct(struct task_struct * current_task, struct task_struct * new_task)
{
	new_task->rip = current_task->rip;

	copy_vma_info(current_task->mm, new_task->mm);



}

void copy_vma_info(struct mm_struct * current_task_mm_s, struct mm_struct * new_task_mm_s)
{
	struct vm_area_struct * current_vma = current_task_mm_s->mmap;

	struct vm_area_struct * prev_new_vma = NULL;

	if (current_vma == NULL)
	{
		new_task_mm_s->mmap = NULL;
		return;
	}

	while(current_vma != NULL)
	{
		struct vm_area_struct *new_vma = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));
		new_vma->vm_start = current_vma->vm_start;
		new_vma->vm_end = current_vma->vm_end;
		new_vma->vm_flags = current_vma->vm_flags;
		new_vma->vm_pgoff = current_vma->vm_pgoff;
		new_vma->vm_file_file_start = current_vma->vm_file_file_start;
		new_vma->vm_file_vm_pgoff = current_vma->vm_file_vm_pgoff;
		new_vma->vm_file_vm_sz = current_vma->vm_file_vm_sz;

		if (prev_new_vma == NULL)
		{
			new_vma->vm_next = NULL;
		}
		else
		{
			prev_new_vma->vm_next = new_vma;
		}

		if (new_task_mm_s->mmap == NULL)
		{
			new_task_mm_s->mmap = new_vma;
		}

		prev_new_vma = new_vma;
		current_vma = current_vma->vm_next;
	}
}

int free_task_struct(struct task_struct * exit_task)
{
	struct task_struct * delete_task = exit_task;
	if (exit_task == NULL)
		return 0;
	if(exit_task->next_task != NULL)
	{
		/*struct task_struct * cur_header = head;
		while(cur_header->next_task != NULL && cur_header->next_task != delete_task)
			cur_header = cur_header->next_task;
		cur_header->next_task = delete_task->next_task;*/

		allProcesses = removeFromTaskList(allProcesses, delete_task);
		//schedule();
	}

	//first set the state of process as TASK_STOPPED
	delete_task->state = TASK_STOPPED;

	//free vma's
	struct mm_struct * delete_mm_struct = exit_task->mm;
	if(delete_mm_struct == NULL)
		return 0;

	struct vm_area_struct *delete_vma = delete_mm_struct->mmap;	//vma list
	struct vm_area_struct *current_vma = delete_vma;
	while(delete_vma != NULL)
	{
		// free vma start's and end's
		current_vma = current_vma->vm_next;
		kfree((uint64_t)delete_vma->vm_start,delete_vma->vm_end-delete_vma->vm_start);

		// free vma itself
		kfree((uint64_t)delete_vma, sizeof(struct vm_area_struct));

		delete_vma = current_vma;
	}

	//free mm_struct
	kfree((uint64_t)delete_mm_struct,sizeof(struct mm_struct));

	delete_task->mm->mmap = NULL;
	delete_task->mm = NULL;

	return 0;
}

uint16_t IsValidFile(char *filename)
{
	uint16_t rip = -1;

	struct elf64_eheader *elf_ehdr;

	struct posix_header_ustar *elf_header = (struct posix_header_ustar *)readTarfile(filename);

	if(elf_header == NULL)
		return -1;

	elf_ehdr = (struct elf64_eheader *)(elf_header+1);

	rip = elf_ehdr->e_entry; 

	if(rip != -1)
		return 1;
	else
		return -1;
}