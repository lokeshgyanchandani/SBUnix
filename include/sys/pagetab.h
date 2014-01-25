#ifndef _PAGETAB_H
#define _PAGETAB_H

#include <defs.h>

#define PR 0x1			//Present
#define RW 0x2			//Read-0/Write-1
#define US 0x4			//User -1/Supervisor -0
#define PWT 0x8			//PageWriteThrough
#define PCD 0x10		//PageCahceDisable
#define A 0x20			//Accessed
#define DI 0x40			//Dirty
#define PAT 0x80		//PageAttributeTable
#define G 0x100			//Global
#define NX 0x8000000000000	//NoExecute

struct PML4
{
	uint64_t entries[512];
};

struct PDP
{
	uint64_t entries[512];
};

struct PD
{
	uint64_t entries[512];
};

struct PT
{
	uint64_t entries[512];
};

#define RECURSIVE_SLOT (510L)

#define L4_SHIFT (39)
#define L3_SHIFT (30)
#define L2_SHIFT (21)
#define L1_SHIFT (12)


#define UPPER_ADDR(x) ((uint64_t*)(0xffffL<<48|(x)))

#define PT_ADDR UPPER_ADDR((RECURSIVE_SLOT<<L4_SHIFT))

#define PD_ADDR UPPER_ADDR((RECURSIVE_SLOT<<L4_SHIFT) \
                     |(RECURSIVE_SLOT<<L3_SHIFT))

#define PDP_ADDR UPPER_ADDR((RECURSIVE_SLOT<<L4_SHIFT) \
                     |(RECURSIVE_SLOT<<L3_SHIFT) \
                     |(RECURSIVE_SLOT<<L2_SHIFT))

#define PML4_ADDR UPPER_ADDR((RECURSIVE_SLOT<<L4_SHIFT) \
                     |(RECURSIVE_SLOT<<L3_SHIFT) \
                     |(RECURSIVE_SLOT<<L2_SHIFT) \
                     |(RECURSIVE_SLOT<<L1_SHIFT))

#define PHY_TO_VIRT_PML4_BASE(type) ((type) ((uint64_t) PML4_ADDR))
#define PHY_TO_VIRT_PDP_BASE(virt, type) ((type) ((uint64_t)PDP_ADDR | (uint64_t)(( ((virt<<16)>>16)>>L4_SHIFT)<<L1_SHIFT)))
#define PHY_TO_VIRT_PD_BASE(virt, type) ((type) ((uint64_t) PD_ADDR | (uint64_t)(( ((virt<<16)>>16)>>L3_SHIFT)<<L1_SHIFT)))
#define PHY_TO_VIRT_PT_BASE(virt, type) ((type) ((uint64_t) PT_ADDR | (uint64_t)(( ((virt<<16)>>16)>>L2_SHIFT)<<L1_SHIFT)))


#define PHY_TO_VIRT_PML4(virt, type) ((type) ((uint64_t) PDP_ADDR | (uint64_t) (( ((virt<<16)>>16)>>L4_SHIFT)<<L1_SHIFT)))
#define PHY_TO_VIRT_PDP(virt, type) ((type) ((uint64_t)PDP_ADDR | (uint64_t)(( ((virt<<16)>>16)>>L3_SHIFT)<<L1_SHIFT)))
#define PHY_TO_VIRT_PD(virt, type) ((type) ((uint64_t) PD_ADDR | (uint64_t)(( ((virt<<16)>>16)>>L2_SHIFT)<<L1_SHIFT)))
#define PHY_TO_VIRT_PT(virt, type) ((type) ((uint64_t)virt))





#define PAGE_TABLE_SIZE 512

#define PAGE_SIZE 0x1000

#define PAGE_MAX 65536

#define MAX_PHY_MEM (PAGE_MAX * PAGE_SIZE)

/*** all available physical memory will be mapped to virtual address (physical + VIRT_OFFSET) ***/
#define VIRT_OFFSET 0xFFFFFFFF80000000

/*** pointer arithmetic ***/
#define PHY_TO_VIRT(phy, type) ((type) ((uint64_t) phy + VIRT_OFFSET))

#define VIRT_BASE 0xFFFFFFFFF0000000

#define PAGE_ALIGN 0xFFFFFFFFFFFFF000

extern void _cpu_write_cr3(uint64_t val);

void * setup_page_tables(uint64_t physbase,uint64_t physfree);

struct PT* getPT(struct PML4 *pml4, uint64_t virt_addr);
void mmap(uint64_t virt_addr,uint64_t phy_addr);
uint64_t  getNewPML4ForUserProcess();
void setCR3(struct PML4 *pml4);
uint64_t getCR3();
void kfree(uint64_t k_virt_start_addr, uint64_t size);
void copy_page_tables(uint64_t pml4_current, uint64_t pml4_new);
void empty_page_tables(uint64_t pml4_t);
uint64_t getMappedPhysicalAddr(uint64_t virt_addr);

uint16_t GetPTIndex(uint64_t lAddress);
uint16_t GetPDIndex(uint64_t lAddress);
uint16_t GetPDPIndex(uint64_t lAddress);
uint16_t GetPML4Index(uint64_t lAddress);

#endif
