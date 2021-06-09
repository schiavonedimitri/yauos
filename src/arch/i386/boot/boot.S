#include "arch/i386/cpu/mmu.h"

.section .boot.multiboot2

	header_start:
		# Multiboot2 magic
		.long 0xE85250D6
		# Multiboot2 architecture, 0 for i386
		.long 0
		# Multiboot2 header length
		.long (header_end - header_start)
		# Multiboot2 checksum -(magic + architecture + length)
		.long -(0xe85250d6 + (header_end - header_start))
		# Multiboot2 tags, must be 8 byte aligned and end with a tag of type 0 and size 8
	.align 8
		# Multiboot2 terminating tag type
		.short 0
		# Multiboot2 terminating tag flags
		.short 0
		# Multiboot2 terminating tag size
		.long 8
	header_end:

# The Multiboot2 spec states that the machine state after loading the kernel is as follows:
# 'EAX': Must contain the magic value  ‘0x36d76289’. The presence of this value indicates to the operating system that it was loaded by a Multiboot2 compliant boot loader.
# 'EBX': Must contain the 32 bit physical address of the Multiboot2 information structure provided by the boot loader.
# 'CS':  Must be a 32 bit read/execute code segment with an offset of '0' and a limit of '0xFFFFFFFF'. The exact value is undefined.
# 'DS','ES','FS','GS','SS': Must be a 32 bit read/write data segment with an offset of '0' and a limit of '0xFFFFFFFF'. The exact values are all undefined.
# 'A20 gate': Must be enabled.
# 'CR0': Bit 31 (PG) must be cleared. Bit 0 (PE) must be set. Other bits are all undefined.
# 'EFLAGS': Bit 17 (VM) must be cleared. Bit 9 (IF) must be cleared. Other bits are all undefined.
# All other processor register and flag bits are undefined. This includes, in particular:
# 'ESP': The OS image must create its own stack as soon as it needs one.
# 'GDTR': Even though the segment registers are set up as described above, the ‘GDTR’ may be invalid, so the OS image must not load any segment registers (even just reloading the same values!) until it sets up its own ‘GDT’.
# 'IDTR': The OS image must leave interrupts disabled until it sets up its own IDT.

.global boot_page_directory
.type boot_page_directory, @object

.section .bss
	.align 4096
	# This is going to be the location of the kernel page directory which will be used throughout the rest of the kernel.
	kernel_directory:
		.skip 4096
	# Location of the first page table (4KB) that maps the first 4MB physical addresses starting from 0x00000000.
	# This memory will be reused in higher layers of the kernel initialization.
	boot_page_table:
		.skip 4096

	.align 16
	# Initial stack for the kernel.
	stack_bottom:
		.skip 4096
	stack_top:

.section .boot.text

	# Main entry point in the elf object.
	.global _start
	.type _start, @function

	_start:
		# Setup the first page table of the kernel to map the first 4MB physical memory starting at physical location 0x00000000.
		# VIRTUAL_TO_PHYSICAL is needed because the kernel is linked at 0xC0100000 but is actually loaded at 0x100000.
		# before paging is enabled manual address fixup is needed.
		movl $VIRTUAL_TO_PHYSICAL(boot_page_table), %edi
		movl $0, %esi
		# Loop only 1023 because the last entry is reserved to map the VGA text mode buffer at 0xB8000 to 0xC03FF000.
		# This is only temporary until the final kernel memory map is actually designed.
		movl $1023, %ecx
	_loop:
		movl %esi, %edx
		orl $0x003, %edx
		movl %edx, (%edi)
		addl $4096, %esi
		addl $4, %edi
		loop _loop
		movl $(0x000B8000 | 0x003), VIRTUAL_TO_PHYSICAL(boot_page_table) + 1023 * 4
		# The first directory entry is needed for identity mapping physical addresses to virtual addresses.
		# This is needed in order to avoid a triple fault on the instruction that jumps to the virtual addresses after enabling paging (which is still in lower addresses).
		movl $(VIRTUAL_TO_PHYSICAL(boot_page_table) + 0x003), VIRTUAL_TO_PHYSICAL(kernel_directory)
		# Actual directory that performs the required mapping to 0xC0000000.
		movl $(VIRTUAL_TO_PHYSICAL(boot_page_table) + 0x003), VIRTUAL_TO_PHYSICAL(kernel_directory) + 768 * 4
		movl $(VIRTUAL_TO_PHYSICAL(kernel_directory)), %ecx
		movl %ecx, %cr3
		movl %cr0, %ecx
		orl $0x80010000, %ecx
		movl %ecx, %cr0
		lea higher_half, %ecx
		# Go to virtual addresses.
		jmp *%ecx

.section .text

	higher_half:
		# Remove identity map because it isn't needed anymore.
		movl $0, kernel_directory
		movl %cr3, %ecx
		movl %ecx, %cr3
		movl $stack_top, %esp
		movl %esp, %ebp
		# The multiboot2 structure was still using the old physical address, fix it.
		addl $(KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE), %ebx
		pushl %ebx
		pushl %eax
		# Kernel entry point
		call kernel_main
		# If for whatever reason the kernel exits, disable interrupts and halt the machine and spin forever (that is so because as per intel manual
		# the halt instruction stops the processor until an enabled interrupt (such as SMI or NMI), the BINIT signal, the INIT signal or the
		# RESET signal is received.
		cli
		loop:
			hlt
			jmp loop
