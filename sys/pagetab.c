#include <sys/pagetab.h>
#include <sys/pmap.h>
#include <stdio.h>

#define PTE_SELF_REF  0xFFFFFF0000000000UL
#define PDE_SELF_REF  0xFFFFFF7F80000000UL
#define PDPE_SELF_REF 0xFFFFFF7FBFC00000UL
#define PML4_SELF_REF 0xFFFFFF7FBFDFE000UL

#define KFLAGS 1 | 2

#define UFLAGS 1 | 2 | 4

#define COW 0x10000000000000

uint64_t READ_ONLY = !RW;


struct PT* getPT(struct PML4 *pml4, uint64_t virt_addr);
uint64_t self_ref(uint64_t pml4e, uint64_t pdpe, uint64_t pde, uint64_t pte);

uint64_t kernel_cr3;

extern char kernmem;
extern void* kernelfree;


uint16_t GetPTIndex(uint64_t lAddress)
{
    return lAddress >> 12 & 0x1FF;
}

uint16_t GetPDIndex(uint64_t lAddress)
{
    return lAddress >> 21 & 0x1FF;
}

uint16_t GetPDPIndex(uint64_t lAddress)
{
    return lAddress >> 30 & 0x1FF;
}

uint16_t GetPML4Index(uint64_t lAddress)
{
    return lAddress >> 39 & 0x1FF;
}

void set_kernel_cr3(uint64_t kern_cr3)
{
	kernel_cr3 = kern_cr3;
}

uint64_t get_kernel_cr3()
{
	return kernel_cr3;
}

void * setup_page_tables(uint64_t kern_physbase, uint64_t kern_physfree)
{
	// Allocate the base page for all the 4 page tables
	struct PML4 *pml4 = (struct PML4 *)kmalloc_mem();
	memset((void *)pml4, 0, PAGE_SIZE);



	//////////////////// MAP KERNEL //////////////////////

	uint64_t kern_virt_addr = (uint64_t)&kernmem;

	struct PT *pt = getPT(pml4, kern_virt_addr);

	uint64_t kern_virt_base_addr = kern_virt_addr & VIRT_BASE;

	uint64_t kern_physbase_t = kern_physbase;
	uint64_t kern_physfree_t = kern_physfree;

	while(kern_physbase_t <= kern_physfree_t)
	{
		uint64_t kern_virt_addr_t = kern_virt_base_addr | kern_physbase_t;	
		pt -> entries[GetPTIndex(kern_virt_addr_t)] = kern_physbase_t | PR | RW | US;
		kern_physbase_t += PAGE_SIZE;
	}


	//////////////////// MAP VIDEO MEMORY //////////////////////

	uint64_t printf_physbase = 0xB8000;
	uint64_t printf_physfree = 0xB8FA0;
	uint64_t printf_virt_addr = PHY_TO_VIRT(printf_physbase, uint64_t);

	pt = getPT(pml4, printf_virt_addr);

	uint64_t printf_virt_base_addr = printf_virt_addr & VIRT_BASE;

	uint64_t printf_physbase_t = printf_physbase;
	uint64_t printf_physfree_t = printf_physfree;

	while(printf_physbase_t <=  printf_physfree_t)
	{
		uint64_t printf_virt_addr_t = printf_virt_base_addr | printf_physbase_t;
		pt -> entries[GetPTIndex(printf_virt_addr_t)] = printf_physbase_t | PR | RW | US;
		printf_physbase_t += PAGE_SIZE;
	}


	//////////////////// MAP PHYSICAL PAGES //////////////////////

	/*uint64_t total_physbase = 0x0;
	uint64_t total_physfree = 0x00000000070fffff;
	uint64_t total_virt_addr;// total_virt_base_addr, total_virt_addr_t;

	uint64_t total_physbase_t = total_physbase;
	uint64_t total_physfree_t = total_physfree;

	while(total_physbase_t <=  total_physfree_t)
	{
		total_virt_addr = PHY_TO_VIRT(total_physbase_t, uint64_t);
		pt = getPT(pml4, total_virt_addr);
		//total_virt_base_addr = total_virt_addr & VIRT_BASE;
		//total_virt_addr_t = total_virt_base_addr | total_physbase_t;
		pt -> entries[GetPTIndex(total_virt_addr)] = total_physbase_t | PR | RW | US;
		total_physbase_t += PAGE_SIZE;
	}*/


	printf("Before setting CR3\n");
	pml4 -> entries[510] = (uint64_t)pml4 | PR | RW | US;

	printf("PML4-> entries[PML4_OFFSET]: %x\n", pml4 -> entries[510]);

	set_kernel_cr3((uint64_t)pml4);
	_cpu_write_cr3((uint64_t)pml4);

	remap_printf();
	remap_bitmap();

	printf("After setting CR3\n\nDONE!!!!\n\n");

	return (struct PML4 *)PHY_TO_VIRT_PML4_BASE(uint64_t);
}

struct PT* getPT(struct PML4 *pml4, uint64_t virt_addr)
{
	uint64_t pml4Index = GetPML4Index(virt_addr);
    	uint64_t pdpIndex = GetPDPIndex(virt_addr);
	uint64_t pdIndex = GetPDIndex(virt_addr);

    	uint64_t pdp = pml4 -> entries[pml4Index] & PAGE_ALIGN;
    	if (!pdp)
    	{
       	struct PDP *newpage = (struct PDP *) kmalloc_mem();
		kernelfree = kernelfree + PAGE_SIZE;
		memset((void *)newpage, 0, PAGE_SIZE);
        	pml4 -> entries[pml4Index] = ((uint64_t)newpage) | PR | RW | US;
		
        	pdp = ((uint64_t)newpage);

    	}

	uint64_t pd = ((struct PDP *)pdp) -> entries[pdpIndex] & PAGE_ALIGN;
    	if (!pd)
    	{
       	struct PD *newpage = (struct PD *) kmalloc_mem();
		kernelfree = kernelfree + PAGE_SIZE;
		memset((void *)newpage, 0, PAGE_SIZE);
        	((struct PDP *)pdp) -> entries[pdpIndex] = ((uint64_t)newpage) | PR | RW | US;

        	pd = ((uint64_t)newpage);

    	}

	uint64_t pt = ((struct PD *)pd) -> entries[pdIndex] & PAGE_ALIGN;
    	if (!pt)
    	{
		struct PT *newpage = (struct PT *) kmalloc_mem();
		kernelfree = kernelfree + PAGE_SIZE;
		memset((void *)newpage, 0, PAGE_SIZE);
       	((struct PD *)pd) -> entries[pdIndex] = ((uint64_t)newpage) | PR | RW | US;

	      	pt = ((uint64_t)newpage);

	}

    	return (struct PT *) pt;
}

void mmap(uint64_t virt_addr,uint64_t phy_addr)
{
	//printf("Inside mmap");
	uint64_t pml4Index = virt_addr >> 39 & 0x1FF;
    	uint64_t pdp = ((struct PML4 *)PHY_TO_VIRT_PML4_BASE(uint64_t))->entries[pml4Index] & PAGE_ALIGN;
    	if (!pdp)
    	{
       	struct PDP *newpage = (struct PDP *) kmalloc_mem();
		kernelfree = kernelfree + PAGE_SIZE;
//		memset((void *)newpage, 0, PAGE_SIZE);
        	((struct PML4 *)PHY_TO_VIRT_PML4_BASE(uint64_t))->entries[pml4Index] = ((uint64_t)newpage) | PR | RW | US;
		
        	pdp = ((uint64_t)newpage);
		//int i=0;
		//for(i=0;i<512;i++)
		//	((struct PDP *)PHY_TO_VIRT_PDP_BASE(virt_addr,uint64_t))->entries[i] = 0;
		
    	}

    	uint64_t pdpIndex = GetPDPIndex(virt_addr);
	uint64_t pd = ((struct PDP *)PHY_TO_VIRT_PDP_BASE(virt_addr,uint64_t))->entries[pdpIndex] & PAGE_ALIGN;
    	if (!pd)
    	{
       	struct PD *newpage = (struct PD *) kmalloc_mem();
		kernelfree = kernelfree + PAGE_SIZE;
//		memset((void *)newpage, 0, PAGE_SIZE);
        	((struct PDP *)PHY_TO_VIRT_PDP_BASE(virt_addr,uint64_t))->entries[pdpIndex] = ((uint64_t)newpage) | PR | RW | US;

        	pd = ((uint64_t)newpage);
		//int i=0;
		//for(i=0;i<512;i++)
		//	((struct PD *)PHY_TO_VIRT_PD_BASE(virt_addr,uint64_t))->entries[i] = 0;

    	}

	uint64_t pdIndex = GetPDIndex(virt_addr);
	uint64_t pt = ((struct PD *)PHY_TO_VIRT_PD_BASE(virt_addr,uint64_t))->entries[pdIndex] & PAGE_ALIGN;
    	if (!pt)
    	{
		struct PT *newpage = (struct PT *) kmalloc_mem();
		kernelfree = kernelfree + PAGE_SIZE;
//		memset((void *)newpage, 0, PAGE_SIZE);
       	((struct PD *)PHY_TO_VIRT_PD_BASE(virt_addr,uint64_t))->entries[pdIndex] = ((uint64_t)newpage) | PR | RW | US;

	      	pt = ((uint64_t)newpage);
		//int i=0;
		//for(i=0;i<512;i++)
		//	((struct PT *) PHY_TO_VIRT_PT_BASE(virt_addr,uint64_t))->entries[i] = 0;
		
	}

	((struct PT *) PHY_TO_VIRT_PT_BASE(virt_addr,uint64_t))->entries[GetPTIndex(virt_addr)] = phy_addr | PR | RW | US;	
}


uint64_t getNewPML4ForUserProcess()
{
	struct PML4 *pml4New = (struct PML4 *) kmalloc_mem();
	uint64_t* virt_addr=NULL;
       virt_addr = PHY_TO_VIRT(kernelfree,void *);
	kernelfree += PAGE_SIZE;

	mmap((uint64_t)virt_addr, (uint64_t)pml4New);	
	memset((void *)virt_addr, 0, PAGE_SIZE);

	//Mapping Kernel
	((struct PML4 *)(virt_addr))->entries[511] = ((struct PML4 *)PHY_TO_VIRT_PML4_BASE(uint64_t))->entries[511];

	// save current cr3 to restore it later down below
	struct PML4 * current_task_cr3 = (struct PML4 *)getCR3();

	// set new cr3 for self-referencing
	setCR3(pml4New);

	//Self-referencing
	((struct PML4 *)(virt_addr))->entries[510] = (uint64_t)pml4New | PR | RW | US;

	// restore current cr3
	setCR3(current_task_cr3);

	return (uint64_t)pml4New;
}

void setCR3(struct PML4 *pml4)
{
	asm volatile("movq %0, %%cr3":: "b"(pml4));
}

uint64_t getCR3()
{
	uint64_t cr3Value;
    	asm volatile("mov %%cr3, %0" : "=r" (cr3Value));
	return cr3Value;
}

uint64_t self_ref(uint64_t pml4e, uint64_t pdpe, uint64_t pde, uint64_t pte)
{
	uint64_t base;
	base = 0xFFFF000000000000; 
	base = (((base >> (12+9+9+9+9))<<9 | pml4e ) << (12+9+9+9) );
	base = (((base >> (12+9+9+9))<<9   | pdpe  ) << (12+9+9) );
	base = (((base >> (12+9+9))<<9     | pde   ) << (12+9) );
	base = (((base >> (12+9))<<9       | pte   ) << (12) );
	return base;
}

void copy_page_tables(uint64_t pml4_current, uint64_t pml4_new)
{
	//uint64_t virt_kern_addr_start = (uint64_t)&kernmem;

	//uint64_t pml4_kern_max = GetPML4Index(virt_kern_addr_start);

	uint64_t pml4Index;
	for (pml4Index = 0; pml4Index < 510; pml4Index++)
	{
		/*if (pml4Index >= 510) // kernel // clone
		{
			setCR3((struct PML4 *)pml4_current);
			uint64_t pml4_entry_current = ((struct PML4 *)PHY_TO_VIRT_PML4_BASE(uint64_t))->entries[pml4Index];

			setCR3((struct PML4 *)pml4_new);
			((struct PML4 *)PHY_TO_VIRT_PML4_BASE(uint64_t))->entries[pml4Index] = pml4_entry_current;
			continue;
		}*/

		setCR3((struct PML4 *)pml4_current);
		uint64_t pdp = ((struct PML4 *)PHY_TO_VIRT_PML4_BASE(uint64_t))->entries[pml4Index];

		if ((pdp & PR) == PR)
		{

			setCR3((struct PML4 *)pml4_new);
			struct PDP * pdp_new = (struct PDP *) kmalloc_mem();
			kernelfree = kernelfree + PAGE_SIZE;
			((struct PML4 *)PHY_TO_VIRT_PML4_BASE(uint64_t))->entries[pml4Index] = ((uint64_t)pdp_new) | UFLAGS;

			setCR3((struct PML4 *)pml4_current);
			uint64_t pdpIndex;
			for (pdpIndex = 0; pdpIndex < 512; pdpIndex++)
			{
				uint64_t pd = ((struct PDP *)self_ref(0x1FE, 0x1FE, 0x1FE, pml4Index))->entries[pdpIndex];

				if ((pd & PR) == PR)
				{
					setCR3((struct PML4 *)pml4_new);
					struct PD * pd_new = (struct PD *) kmalloc_mem();
					kernelfree = kernelfree + PAGE_SIZE;
					((struct PDP *)self_ref(0x1FE, 0x1FE, 0x1FE, pml4Index))->entries[pdpIndex] = (uint64_t)((uint64_t)pd_new | UFLAGS);

					setCR3((struct PML4 *)pml4_current);
					uint64_t pdIndex;
					for (pdIndex = 0; pdIndex < 512; pdIndex++)
					{
						uint64_t pt = ((struct PD *)self_ref(0x1FE, 0x1FE, pml4Index, pdpIndex))->entries[pdIndex];

						if((pt & PR) == PR)
						{
							setCR3((struct PML4 *)pml4_new);
							struct PT * pt_new = (struct PT *) kmalloc_mem();
							kernelfree = kernelfree + PAGE_SIZE;
							((struct PD *)self_ref(0x1FE, 0x1FE, pml4Index, pdpIndex))->entries[pdIndex] = (uint64_t)((uint64_t)pt_new | UFLAGS);

							setCR3((struct PML4 *)pml4_current);

							uint64_t ptIndex;
							for (ptIndex = 0; ptIndex < 512; ptIndex++)
							{
								uint64_t page = ((struct PT *) self_ref(0x1FE, pml4Index, pdpIndex, pdIndex))->entries[ptIndex];

								if ((page & PR) == PR)
								{
									page = page & PAGE_ALIGN;
									//if (pdIndex == 448)
									//	page = page  | PR | RW | US;
									//else
										page = (page | COW)  | PR | !RW | US;

									// same physical address in the new PT
									setCR3((struct PML4 *)pml4_new);
									((struct PT *) self_ref(0x1FE, pml4Index, pdpIndex, pdIndex))->entries[ptIndex] = page;

									// update address in the old PT
									setCR3((struct PML4 *)pml4_current);
									((struct PT *) self_ref(0x1FE, pml4Index, pdpIndex, pdIndex))->entries[ptIndex] = page;
								}
							}
						}
					}
				}
			}
		}
	}

	//return (void *)pml4_new;
}

void empty_page_tables(uint64_t pml4_t)
{
	uint64_t pml4Index, pdpIndex, pdIndex, ptIndex;

	uint64_t current_cr3 = getCR3();

	setCR3((struct PML4 *)pml4_t);

	// Free entries except [510] and [511] entries
	for (pml4Index = 0; pml4Index < 510; pml4Index++)
	{

		uint64_t pdp = ((struct PML4 *)PHY_TO_VIRT_PML4_BASE(uint64_t))->entries[pml4Index];
		if ((pdp & PR) == PR)
		{

			for (pdpIndex = 0; pdpIndex < 512; pdpIndex++)
			{

				uint64_t pd = ((struct PDP *)self_ref(0x1FE, 0x1FE, 0x1FE, pml4Index))->entries[pdpIndex];
				if ((pd & PR) == PR)
				{

					for (pdIndex = 0; pdIndex < 512; pdIndex++)
					{

						uint64_t pt = ((struct PD *)self_ref(0x1FE, 0x1FE, pml4Index, pdpIndex))->entries[pdIndex];
						if((pt & PR) == PR)
						{
							for (ptIndex = 0; ptIndex < 512; ptIndex++)
							{

								uint64_t page = ((struct PT *) self_ref(0x1FE, pml4Index, pdpIndex, pdIndex))->entries[ptIndex];
								if ((page & PR) == PR && ((COW & page) != COW))
								{
									page = (page & PAGE_ALIGN);
									((struct PT *)self_ref(0x1FE, pml4Index, pdpIndex, pdIndex))->entries[ptIndex] = 0;
									set_page_free(page / PAGE_SIZE);
								}
							}
						}
					}
				}
			}
		}
	}

	//TODO: Need to zero out [510] and [511] by using virtual address of pml4_t
	setCR3((struct PML4 *)current_cr3);
	//set_page_free((pml4_t & PAGE_ALIGN) / PAGE_SIZE);
}

uint64_t getMappedPhysicalAddr(uint64_t virt_addr)
{
	uint64_t phy_addr = 0;

	uint64_t pml4Index = GetPML4Index(virt_addr);
    	uint64_t pdp = ((struct PML4 *)PHY_TO_VIRT_PML4_BASE(uint64_t))->entries[pml4Index];
    	if ((pdp & PR) == PR)
    	{
    		uint64_t pdpIndex = GetPDPIndex(virt_addr);
		uint64_t pd = ((struct PDP *)PHY_TO_VIRT_PDP_BASE(virt_addr,uint64_t))->entries[pdpIndex];

    		if ((pd & PR) == PR)
    		{
			uint64_t pdIndex = GetPDIndex(virt_addr);
			uint64_t pt = ((struct PD *)PHY_TO_VIRT_PD_BASE(virt_addr,uint64_t))->entries[pdIndex];
		    	if ((pt & PR) == PR)
    			{
				uint64_t ptIndex = GetPTIndex(virt_addr);
				phy_addr = ((struct PT *) PHY_TO_VIRT_PT_BASE(virt_addr,uint64_t))->entries[ptIndex];
			}
		}
	}

	return phy_addr;
}

void kfree(uint64_t k_virt_start_addr, uint64_t size)
{
	uint64_t virt_start_addr = k_virt_start_addr;

	int times = (size % PAGE_SIZE == 0) ? (size / PAGE_SIZE) : (size / PAGE_SIZE) + 1;
	while(times > 0)
	{
		uint64_t phys_addr = getMappedPhysicalAddr(virt_start_addr);

		if ((phys_addr & PR) == PR && ((COW & phys_addr) != COW))
		{
			uint64_t ptIndex = GetPTIndex(virt_start_addr);
			((struct PT *) PHY_TO_VIRT_PT_BASE(virt_start_addr,uint64_t))->entries[ptIndex] = 0;
			set_page_free((phys_addr & PAGE_ALIGN) / PAGE_SIZE);
		}
		virt_start_addr += PAGE_SIZE;
		times--;
	}

	//again set the free bit in freeList

	//set the pt entry to zero for the address
};