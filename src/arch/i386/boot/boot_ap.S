# Please note that this project uses the AT&T assembly syntax.

#include <arch/cpu/cpu.h>
#include <arch/cpu/gdt.h>
#include <arch/mmu.h>

# SMP Trampline code:
# The application processors unlike the bootstrap processor are not booted by GRUB (which already enters protected mode for the bootstrap processor
# and sets some initial environment); so they start in real mode. For this reason for the AP CPUs manual initialization must take place:
# - Loading an initial bootstrap GDT and entering protected mode.
# - Enabling paging and switching to the kernel page directory.
# - Jumping to the AP main entry point in the kernel and continue AP initialization there.
# This code is linked as an external binary to the kernel image and is relocated to 0x1000.
# Unlike the BSP this code is linked and loaded at the same address: 0x1000 so no address fixing is needed before enabling paging.
# All that is needed is a bootstrap page directory that identity maps virtual addresses to physical addresses.
# The relocation code puts 3 parameters in the page before this (0x0).
# The first parameter at offset - 4 from this entry point is the page directory this CPU should use.
# The second parameter at offset - 8 from this entry point is the allocated stack for this CPU.
# Note: the allocated stack address passed is already a virtual address and not a physical one.
# The third and last parameter at offset - 12 from this entry point is the entry point address this CPU should jump to.
# Note: As for the parameters passed to this code, the physical memory manager already reserves page 0 to preserve the 
# real mode IVT and BDA area. As on a paged system it is possible to only reserve whole pages this means that the parameters passed to this routine
# which reside at the end of page 0 do exist if SMP is detected during initialization.
# Also note that the real mode IVT does not use the whole 0th page so it's completely fine to pass parameters at the end of this page.
# Note that we are talking about physical pages here, virtual page 0 will not be mapped in the kernel and in the userland to 
# permit catching NULL pointer references.

.section .text
.code16
.globl _ap_start16
_ap_start16:
    
    # Disable interrupts and clear direction flag.
    
    cli
    cld
    
    # - Load the bootstrap GDT.
    
    lgdtl gdtdescriptor
    
    # - Enable protected mode.
    
    movl %cr0, %eax
    
    # - Enable memory caching and write through caching like on the BSP processor.
    
    andl $(~((1 << CR0_MEMORY_CACHE_SHIFT) | (1 << CR0_WRITE_THROUGH_CACHING_SHIFT))), %eax
    orl $CR0_PROTECTED_MODE_ENABLED, %eax
    movl %eax, %cr0
    
    # Reload segment selectors:
    # Do a far jump to reaload the code segment selector with the new GDT code segment selector and start running 32 bit code.
    
    ljmp $(GDT_KERNEL_CODE_OFFSET), $_ap_start32

.code32
_ap_start32:
    
    # Reload the remaining segment selectors with the new GDT data segment selector.
    
    movw $GDT_KERNEL_DATA_OFFSET, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    
    # Load Kernel directory from the passed parameters.
    
    movl _ap_start16 - 4, %eax
    
    # Set CR3 to point to the kernel directory.
    
    movl %eax, %cr3
    
    # Enable paging and WP bit to enable write protection for Kernel pages.
    
    movl %cr0, %eax
    orl $(CR0_PAGING_ENABLED | CR0_WRITE_PROTECT_ENABLED), %eax
	movl %eax, %cr0
    
    # Load stack pointer from the passed parameters.
    
    movl _ap_start16 - 8, %eax
    
    # Setup the stack pointer.
    
    movl %eax, %esp
    
    # Setup initial stack frame.
    
    movl %esp, %ebp
    
    # Push local apic identifier of this cpu.
    
    movl _ap_start16 - 16, %eax
    pushl %eax
    
    # Call CPU main entry point from the passed parameters.
    
    call *(_ap_start16 - 12)
   	
    # If for whatever reason the CPU returns, disable interrupts and halt the machine and spin forever (that is so because as per intel manual
	# the halt instruction stops the processor until an enabled interrupt (such as SMI or NMI), the BINIT signal, the INIT signal or the
	# RESET signal is received.
    # One might assume it is not safe to return here because the relocated code might not exist anymore. This is not the case
    # because the physical memory manager reserves this page if SMP is detected during initialization.
	
    cli
1:
	hlt
	jmp 1b

.section .rodata

# Bootstrap GDT for the processor.

.align 8
gdt:
    
    # Null segment as per Intel manual.
    
    SEGMENT_NULL
    
    # Flat kernel code segment.
    
    SEGMENT_KCODE(0, 0xFFFFFFFF)
    
    # Flat kernel data segment.
    
    SEGMENT_KDATA(0, 0xFFFFFFFF)

# GDT descriptor.

gdtdescriptor:
    
    # Sizeof(gdtdescriptor) - 1
    
    .word gdtdescriptor - gdt - 1
    
    # Gdt pointer.
    
    .long gdt