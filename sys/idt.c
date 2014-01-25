#include <sys/io.h>
#include <stdio.h>
#include <sys/idt.h>

static void init_idt();
idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;

extern void _load_idt();

void reload_idt()
{
    init_idt();
};

static void init_idt()
{
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base = (uint64_t)&idt_entries;
    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

     init_isr();
     init_irq();
    _load_idt(&idt_ptr);
};

void idt_set_gate(int num,uint16_t selector, unsigned ist, unsigned type, unsigned dpl, uint64_t offset)
{
    idt_entries[num].offset_low = (offset) & 0xFFFF;
    idt_entries[num].selector = selector;
    idt_entries[num].ist = ist;
    idt_entries[num].reserved0 = 0;
    idt_entries[num].type = type;
    idt_entries[num].zero = 0;
    idt_entries[num].dpl = dpl;
    idt_entries[num].p = 1;
    idt_entries[num].offset_mid = (offset >> 16) & 0xFFFF;
    idt_entries[num].offset_high = (offset >> 32) & 0xFFFFFFFF;
};
