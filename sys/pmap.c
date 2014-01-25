#include <sys/pmap.h>
#include <sys/pagetab.h>
#include <stdio.h>
#include <math.h>
#include <sys/sched.h>

#define PAGE_SIZE 0x1000
#define MAX_PAGES 8192
#define userheap 0x1000000

#define __GFP_WAIT	0x10	/* Can wait and reschedule? */
#define __GFP_HIGH	0x20	/* Should access emergency pools? */
#define __GFP_IO	0x40	/* Can start physical IO? */
#define __GFP_FS	0x80	/* Can call down to low-level FS? */
#define __GFP_COLD	0x100	/* Cache-cold page required */
#define __GFP_NOWARN	0x200	/* Suppress page allocation failure warning */
#define __GFP_REPEAT	0x400	/* Retry the allocation.  Might fail */
#define __GFP_NOFAIL	0x800	/* Retry for ever.  Cannot fail */
#define __GFP_NORETRY	0x1000	/* Do not retry.  Might fail */
#define __GFP_NO_GROW	0x2000	/* Slab internal usage */
#define __GFP_COMP	0x4000	/* Add compound page metadata */

#define __GFP_BITS_SHIFT 16	/* Room for 16 __GFP_FOO bits */
#define __GFP_BITS_MASK ((1 << __GFP_BITS_SHIFT) - 1)

/* if you forget to add the bitmask here kernel will crash, period */
#define GFP_LEVEL_MASK (__GFP_WAIT|__GFP_HIGH|__GFP_IO|__GFP_FS| \
			__GFP_COLD|__GFP_NOWARN|__GFP_REPEAT| \
			__GFP_NOFAIL|__GFP_NORETRY|__GFP_NO_GROW|__GFP_COMP)

#define GFP_ATOMIC	(__GFP_HIGH)
#define GFP_NOIO	(__GFP_WAIT)
#define GFP_NOFS	(__GFP_WAIT | __GFP_IO)
#define GFP_KERNEL	(__GFP_WAIT | __GFP_IO | __GFP_FS)
#define GFP_USER	(__GFP_WAIT | __GFP_IO | __GFP_FS)
#define GFP_HIGHUSER	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HIGHMEM)

/* Flag - indicates that the buffer will be suitable for DMA.  Ignored on some
   platforms, used as appropriate on others */

#define GFP_DMA		__GFP_DMA



unsigned char *free_pages_list;
extern void* kernelbase;
extern void* kernelfree;
uint64_t userh;

void set_page_free(uint64_t page)
{
	if (page)
		free_pages_list[(uint64_t)page / 8] &= ~(1 << (page % 8));
}

void set_page_used(uint64_t page)
{
	if (page)
		free_pages_list[(uint64_t)page / 8] |= (1 << (page % 8));
}

void init_freelist(void* physbase, void* physfree)
{
	userh = userheap;
	kernelbase = physbase;
	kernelfree = physfree;

	free_pages_list = (unsigned char *)physfree;

	uint64_t counter;
	for (counter = 0; counter < MAX_PAGES; counter ++)
	{
		free_pages_list[counter] = 255;
	}
	kernelfree += (uint64_t)(sizeof(char) * MAX_PAGES);
}

bool is_memory_reserved(uint64_t page)
{
	if (page * PAGE_SIZE >= (uint64_t)kernelbase && page * PAGE_SIZE <= (uint64_t)kernelfree)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void set_kernel_in_freelist(uint64_t start_addr, uint64_t end_addr)
{
	uint64_t page = start_addr / PAGE_SIZE;
	uint64_t end_page = end_addr / PAGE_SIZE;

	for (; page < end_page; page++)
	{
		if (is_memory_reserved(page))
		{		
			free_pages_list[(uint64_t)page / 8] |= 1 << (page % 8);
		}
		else
		{
			free_pages_list[(uint64_t)page / 8] &= ~(1 << (page % 8));
		}
	}
}

/*
void reserve_pages(uint64_t start_addr, uint64_t end_addr)
{
	uint64_t page = start_addr / PAGE_SIZE;
	uint64_t end_page = end_addr / PAGE_SIZE;

	for (; page < end_page; page++)
	{
		free_pages_list[(uint64_t)page / 8] &= ~(1 << (page % 8));
	}
}*/


//if you are using kmalloc_mem..it's ur responsiblity to increase the kernelfree
void* kmalloc_mem()
{
	uint64_t page = 528;		
	while (free_pages_list[page] == 255) page++;	
	for(; page < MAX_PAGES; page++)
	{
		if(!(free_pages_list[(uint64_t)page / 8] >> (page % 8)) & 0x01)
		{
			free_pages_list[(uint64_t)page / 8] |= 1 << (page % 8);
			uint64_t address = page * PAGE_SIZE;
			//kernelfree = kernelfree + PAGE_SIZE;
			return (void *)address;
		}
	}
	return NULL;

}

//for allocating a complete page
void* kmalloc_page()
{
    uint64_t* virt_addr=NULL;
    uint64_t* phys_addr=NULL;
    
     virt_addr = PHY_TO_VIRT(kernelfree,void *);
     phys_addr = kmalloc_mem();
     mmap((uint64_t)virt_addr,(uint64_t)phys_addr);     
     kernelfree = kernelfree + PAGE_SIZE;
     return virt_addr;
	
};


//for allocating pages according to the size_mem parameter
void* kmalloc(uint64_t size_mem)
{
    int num = 0;
    if(size_mem%PAGE_SIZE == 0)
	 num = size_mem/PAGE_SIZE;
    else
	num = size_mem/PAGE_SIZE+1;
	
    uint64_t* virt_addr=NULL;
    uint64_t* phys_addr=NULL;
    uint32_t  c;
    bool isBase = 0;
    uint64_t *base_addr = NULL;
    

    for(c=0;c<num;++c){
	virt_addr = PHY_TO_VIRT(kernelfree,void *);
	
	if(!isBase){
		base_addr = virt_addr;
		isBase = 1;
	}
    	phys_addr = kmalloc_mem();
    	mmap((uint64_t)virt_addr,(uint64_t)phys_addr);     
    	kernelfree = kernelfree + PAGE_SIZE;
    }
    return base_addr;   
	
};


//for allocating pages according to the size_mem parameter and at a given virtual address
void* kmalloc_virtual(uint64_t size_mem,uint64_t virt)
{
    int num;
	if (size_mem % PAGE_SIZE == 0)
	{
		num = size_mem/PAGE_SIZE;
	}
	else
	{
		num = size_mem/PAGE_SIZE + 1;
	}

    uint64_t virt_addr=(uint64_t)virt;
    uint64_t* phys_addr=NULL;
    uint32_t  c;
    bool isBase = 0;
    uint64_t *base_addr = NULL;
    

    for(c=0;c<num;++c){
	//virt_addr = PHY_TO_VIRT(kernelfree,void *);
	
	if(!isBase){
		base_addr = (uint64_t *)virt_addr;
		isBase = 1;
	}
    	phys_addr = kmalloc_mem();
    	mmap((uint64_t)virt_addr,(uint64_t)phys_addr);     
    	kernelfree = kernelfree + PAGE_SIZE;
	virt_addr = (uint64_t)virt_addr + (uint64_t)PAGE_SIZE;
    }
    return base_addr;   
	
};


void* sbrk_mem()
{
	uint64_t page = 100;	//we can assign from 0
	while (free_pages_list[page] == 255) page++;

	for(; page < MAX_PAGES; page++)
	{
		if(!(free_pages_list[(uint64_t)page / 8] >> (page % 8)) & 0x01)
		{

			free_pages_list[(uint64_t)page / 8] |= 1 << (page % 8);
			uint64_t address = page * PAGE_SIZE;
			return (void *)address;
		}
	}
	return NULL;
}


void* sbrk(uint64_t size_mem)
{
    int num = 0;
    if(size_mem%PAGE_SIZE == 0)
	 num = size_mem/PAGE_SIZE;
    else
	num = size_mem/PAGE_SIZE+1;
	
    uint64_t virt_addr=0;
    uint64_t* phys_addr=NULL;
    uint32_t  c;
    bool isBase = 0;
    uint64_t *base_addr = NULL;
    

    for(c=0;c<num;++c){
	//virt_addr = PHY_TO_VIRT(kernelfree,void *);
	if (current_task)
		virt_addr = (uint64_t)current_task->cur_heap;
	else
		virt_addr = (uint64_t)_UHEAP+c*PAGE_SIZE;
	if(!isBase){
		base_addr = (uint64_t *)virt_addr;
		isBase = 1;
	}
    	phys_addr = kmalloc_mem();
    	mmap(virt_addr,(uint64_t)phys_addr);
	if (current_task)
		current_task->cur_heap = current_task->cur_heap + PAGE_SIZE;     	
    	//kernelfree = kernelfree + PAGE_SIZE;
    }
    return base_addr;   	
};


void* sbrk_s(uint64_t size_mem)
{
    int num = 0;
    if(size_mem%PAGE_SIZE == 0)
	 num = size_mem/PAGE_SIZE;
    else
	num = size_mem/PAGE_SIZE+1;
	
    uint64_t virt_addr=0;
    uint64_t* phys_addr=NULL;
    uint32_t  c;
    bool isBase = 0;
    uint64_t *base_addr = NULL;
    

    for(c=0;c<num;++c){
	//virt_addr = PHY_TO_VIRT(kernelfree,void *);
		virt_addr = (uint64_t)userh;
	if(!isBase){
		base_addr = (uint64_t *)virt_addr;
		isBase = 1;
	}
    	phys_addr = sbrk_mem(); //kmalloc_mem();
    	mmap(virt_addr,(uint64_t)phys_addr);
    	userh = userh + PAGE_SIZE;
    }
    return base_addr;   	
};



void remap_bitmap()
{
	free_pages_list = PHY_TO_VIRT(free_pages_list, unsigned char *);	
}
