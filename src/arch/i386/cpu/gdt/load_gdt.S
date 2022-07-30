#include <arch/cpu/gdt.h>

.section .text
	.global load_gdt
	.type load_gdt, @function
	load_gdt:
		movl 4(%esp), %eax
		lgdtl (%eax)
		
		# Loading kernel data segment offset which is at 0x10 into the gdt and using it for all data segment selectors.
		
		movw $(GDT_KERNEL_DATA_OFFSET), %ax
		movw %ax, %ds
		movw %ax, %es
		movw %ax, %fs
		movw %ax, %ss
		movw $(GDT_KERNEL_PER_CPU_DATA_OFFSET), %ax
		movw %ax, %gs
		
		# This is necessary to load the code segment selector CS with the kernel code segment, which is at offset 0x8 into the gdt.
		
		ljmpl $0x8, $0f

                0:
                        ret

	.size load_gdt, . - load_gdt