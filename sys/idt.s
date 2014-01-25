.text

######
# load a new IDT
#  parameter 1: address of idtr
.global _load_idt
_load_idt:
    #cli
    lidt (%rdi)
    #sti
    retq                       # far-return to new cs descriptor ( the retq below )
