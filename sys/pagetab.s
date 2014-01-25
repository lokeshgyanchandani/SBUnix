.text

.global _cpu_write_cr3
_cpu_write_cr3:
    movq %rdi, %rax
    movq %rax, %cr3
    retq
