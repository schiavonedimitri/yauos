# Please note that this project uses the AT&T assembly syntax.

#include <arch/cpu/cpu.h>
#include <arch/cpu/gdt.h>
#include <arch/mmu.h>

# defining a separate section for the multiboot2 header because it needs to be linked first in order for a multiboot2 compliant bootloader to boot us. See the linker script.

.section .multiboot2
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
	.align 0x8
		
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

# Make the global directory and initial page table public symbols for the kernel to reuse these memory in later stages.
# Also define public symbols for the kernel early heap based on bss.

.global kernel_directory
.type kernel_directory, @object

.global bootmem_start
.type bootmem_start, @object

.global bootmem_end
.type bootmem_end, @object

.global kernel_boot_page_table
.type kernel_boot_page_table, @object

.section .bss
	
	# This is going to be an area of EARLY_HEAP_SIZE bytes used for memory allocation early in the boot stage. Among other things it will be used to booststrap the physical memory system itself.
	
	.align 4096
	bootmem_start:
	.skip EARLY_HEAP_SIZE
	bootmem_end:
	
	# This is going to be the location of the kernel page directory which will be used throughout the rest of the kernel.
	
	.align 4096
	kernel_directory:
	.skip 4096
	
	# Location of the first page table (4Kb) that maps the first 4Mb physical addresses starting from 0x00000000.
	# This memory will be reused in higher layers of the kernel initialization.
	
	.align 4096
	kernel_boot_page_table:
		.skip 4096
	
	# Initial stack for the kernel.
	
	.align 16
	stack_bottom:
		.skip 4096
	stack_top:

.extern _KERNEL_TEXT_START_
.extern _KERNEL_DATA_START_

.section .text
	
	# Main entry point in the elf object.
	
	.global _start
	.type _start, @function
	_start:
		
		# Setup initial bootstrap GDT as we cannot rely on the one provided by GRUB.
		
		lgdtl VIRTUAL_TO_PHYSICAL(gdtdescriptor)
		
		# Do a far jump to reaload the code segment selector with the new GDT code segment selector.
		
		ljmpl $GDT_KERNEL_CODE_OFFSET, $VIRTUAL_TO_PHYSICAL(1f)
	1:
		
		# Reload the remaining segment selectors with the new GDT data segment selector.
		
		xorl %ecx, %ecx
    	movw $GDT_KERNEL_DATA_OFFSET, %cx
    	movw %cx, %ds
    	movw %cx, %es
    	movw %cx, %fs
    	movw %cx, %gs
    	movw %cx, %ss	
		
		# Zero the general purpose registers but eax and ebx because they contain data left by the bootloader that we need to pass to the Kernel entry point.
		
		xorl %edx, %edx
		movl %edx, %ecx
		movl %edx, %esi
		movl %edx, %edi
		
		# Setup physical stack pointer for use in this code.
		
		movl $VIRTUAL_TO_PHYSICAL(stack_top), %esp
		
		# Setup the first page table of the kernel to map the first 2Mb physical memory starting at physical location 0x00000000.
		# VIRTUAL_TO_PHYSICAL is needed because the kernel is linked at 0xC0100000 but is actually loaded at 0x100000, see the linker script.
		# Manual address fixing is needed because all addresses are virtual but paging is still disabled and we need to work with physical addresses until then.
		# Kernel .text and .rodata sections are mapped read only, while .data and .bss are mapped read/write according to elf.
		# Note: some mappings will change after the memory map is inspected by the memory management init code.
		# 1) Map from 0x0 to _KERNEL_TEXT_START_ R/W.
		
		movl $VIRTUAL_TO_PHYSICAL(kernel_boot_page_table), %edi
		movl $VIRTUAL_TO_PHYSICAL(_KERNEL_TEXT_START_), %ecx
		movl $0, %esi
	1:
		movl %esi, %edx
		orl $0x3, %edx
		movl %edx, (%edi)
		addl $4096, %esi
		addl $4, %edi
		cmpl %ecx, %esi
		jne 1b
		
		# 2) Map from _KERNEL_TEXT_START_ to _KERNEL_DATA_START_ R.
		
		movl $VIRTUAL_TO_PHYSICAL(_KERNEL_DATA_START_), %ecx
	1:
		movl %esi, %edx
		orl $0x1, %edx
		movl %edx, (%edi)
		addl $4096, %esi
		addl $4, %edi
		cmpl %ecx, %esi
		jne 1b
		
		# Calculate how many pages have been mapped of the 512 we wanted to map.
		
		pushl %edi
		subl $VIRTUAL_TO_PHYSICAL(kernel_boot_page_table), %edi
		shr $0x2, %edi
		movl $512, %edx
		subl %edi, %edx
		movl %edx, %ecx
		popl %edi
		
		# 3) Map the remaining pages of the 512 we wanted to map substracted by those already mapped above R/W. This includes the kernel .bss and more pages up to 2Mb so that
		# we don't miss to map the multiboot2 structure and have enough memory mapped for initialization.
	
	1:
		movl %esi, %edx
		orl $0x3, %edx
		movl %edx, (%edi)
		addl $4096, %esi
		addl $4, %edi
		loop 1b
		
		# Remap video display memory (0xA0000 - 0xC0000) with caching disabled.
		# 0xA0000 index is 160 in the page table.
		
		movl $(VIRTUAL_TO_PHYSICAL(kernel_boot_page_table) + 160 * 4), %edi
		movl $0xC0000, %ecx
		movl $0xA0000, %esi
	1:
		movl %esi, %edx
		orl $0x13, %edx
		movl %edx, (%edi)
		addl $4096, %esi
		addl $4, %edi
		cmpl %ecx, %esi
		jne 1b
		
		# The first directory entry is needed for identity mapping physical addresses to virtual addresses.
		# This is needed in order to avoid a triple fault on the instruction that jumps to the virtual addresses after enabling paging (which is still in lower addresses).
		
		movl $(VIRTUAL_TO_PHYSICAL(kernel_boot_page_table) + 0x3), VIRTUAL_TO_PHYSICAL(kernel_directory)
		
		# Actual directory that performs the required mapping to 0xC0000000.
		
		movl $(VIRTUAL_TO_PHYSICAL(kernel_boot_page_table) + 0x3), VIRTUAL_TO_PHYSICAL(kernel_directory) + 768 * 4
		
		# Recurive directory entry.
		
		movl $(VIRTUAL_TO_PHYSICAL(kernel_directory) + 0x3), VIRTUAL_TO_PHYSICAL(kernel_directory) + 1023 * 4
		movl $(VIRTUAL_TO_PHYSICAL(kernel_directory)), %ecx
		
		# Pointing cr3 to the kernel directory just created.
		
		movl %ecx, %cr3
		movl %cr0, %ecx
		
		# Enabling paging and kernel page table protection (so that pages marked as read only for kernel will cause a page fault on write).
		
		orl $(CR0_PAGING_ENABLED | CR0_WRITE_PROTECT_ENABLED), %ecx
		movl %ecx, %cr0
		leal 1f, %ecx
		
		# Go to virtual addresses by using an indirect jump.
		
		jmp *%ecx
	1:
		
		# Remove identity map because it isn't needed anymore.
		
		movl $(CR0_PAGING_ENABLED | CR0_WRITE_PROTECT_ENABLED), kernel_directory
		
		# This is needed to force a TLB flush, otherwise the old identity mapping could persist indefinitely (As per Intel manual).
		
		movl %cr3, %ecx
		movl %ecx, %cr3
		
		# Set initial stack frame (esp is set up again because paging is active now) and call kernel's architecture main entry point.
		
		movl $stack_top, %esp
		movl %esp, %ebp
		
		# The multiboot2 structure was still using the old physical address, fix it.
		
		addl $(KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE), %ebx
		pushl %ebx
		pushl %eax
		
		# architecture entry point
		
		call arch_main
		
		# If for whatever reason the architecture entry point exits, disable interrupts and halt the machine and spin forever (that is so because as per intel manual
		# the halt instruction stops the processor until an enabled interrupt (such as SMI or NMI), the BINIT signal, the INIT signal or the
		# RESET signal is received.
		
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


# GDT descriptor. VIRTUAL_TO_PHYSICAL is needed because the kernel is linked at its virtual base
# but loaded at 0x100000. Before paging is enabled manual address fixing is needed.

gdtdescriptor:
    
	# Sizeof(gdtdescriptor) - 1
    
	.word gdtdescriptor - gdt - 1
    
	# Gdt pointer.
    
	.long VIRTUAL_TO_PHYSICAL(gdt)