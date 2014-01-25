#ifndef _PMAP_H
#define _PMAP_H

#include <defs.h>
#define _USTACK 0xA8000000

#define _UHEAP 0xF8000000

void init_freelist(void* physbase, void* physfree);
void set_kernel_in_freelist(uint64_t start_addr, uint64_t end_addr);
void reserve_pages(uint64_t start_addr, uint64_t end_addr);
void* kmalloc_page();
void* kmalloc(uint64_t size_mem);
void* kmalloc_mem();
void* kmalloc_virtual(uint64_t size_mem,uint64_t virt);
void* sbrk(uint64_t size_mem);
void* sbrk_mem();
void* sbrk_s(uint64_t size_mem);

void remap_bitmap();

void set_page_free(uint64_t page);
void set_page_used(uint64_t page);

#endif
