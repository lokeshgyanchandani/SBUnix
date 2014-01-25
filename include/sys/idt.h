#include <defs.h>

void reload_idt();
struct idt_entry_struct
{
    uint16_t offset_low;
    uint16_t selector;
    unsigned ist : 3;
    unsigned reserved0 : 5;
    unsigned type : 4;
    unsigned zero : 1;
    unsigned dpl : 2;
    unsigned p : 1;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved1;
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

struct idt_ptr_struct
{
    uint16_t limit;
    uint64_t base;
} __attribute__ ((packed));

typedef struct idt_ptr_struct idt_ptr_t;


typedef struct isr_stack_frame
{
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9; 
	uint64_t r8; 
	uint64_t rdi;
	uint64_t rsi; 
	uint64_t rdx; 
	uint64_t rcx;
	uint64_t rbx; 
	uint64_t rax;
	uint64_t rbp;

//	unsigned char err_code;

	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
} isr_stack_frame_t;


struct registers_t
  {
      uint64_t r15,r14,r13,r12,r11,r10;
      uint64_t r9,r8,rdi,rsi,rdx,rcx;
      uint64_t rbx,rax,rbp;
      uint64_t rip,cs,rflags,rsp,ss;
  }__attribute((packed));

void idt_set_gate(int num,uint16_t selector, unsigned ist, unsigned type, unsigned dpl, uint64_t offset);

void init_isr();
void init_irq();
