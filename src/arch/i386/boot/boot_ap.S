#include <arch/cpu/cpu.h>
#include <arch/cpu/gdt.h>
#include <arch/mmu.h>

# Application processors startup code.
# The aps unlike the bootstrap processor are not booted by the bootloader and so they start in real mode.
# For the aps all manual initialization must take place: loading an initial bootsrap gdt and entering protected mode.
# Enabling paging and using the kernel page directory.
.section .text
.code16
.globl _ap_start16
_ap_start16:
    cli
    cld
    # First load the bootstrap gdt.
    lgdtl gdtdesc
    # Enable protected mode.
    movl %cr0, %eax
    orl $CR0_PROTECTED_MODE_ENABLE, %eax
    movl %eax, %cr0
    # Do a far jump to reaload cs to the 32 bit kernel code segment selector.
    ljmp $(GDT_KERNEL_CODE_OFFSET), $_ap_start32

.code32
_ap_start32:
    # Reload the other segments with the 32 bit from the gdt.
    movw $GDT_KERNEL_DATA_OFFSET, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movl _ap_start16 - 8, %eax
    # Set kernel directory.
    movl %eax, %cr3
    # Enable paging.
    movl %cr0, %eax
    orl $0x80010001, %eax
	movl %eax, %cr0
    movl _ap_start16 - 4, %eax
    movl %eax, %esp
    call *(_ap_start16 - 12)
    cli
    hlt

# Bootstrap gdt for the processor.
.align 8
gdt:
    SEGMENT_NULL
    SEGMENT_KCODE(0, 0xFFFFFFFF)
    SEGMENT_KDATA(0, 0xFFFFFFFF)

gdtdesc:
    .word   (gdtdesc - gdt - 1)
    .long   gdt


