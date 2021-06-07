.section .multiboot2

	header_start:
		# Multiboot2 magic
		.long 0xe85250d6
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

.section .bss
	.align 16

	stack_bottom:
		.skip 4096
	stack_top:

.section .text

	.global _start
	.type _start, @function

	_start:
		movl $stack_top, %esp
		pushl %ebx
		pushl %eax
		# Kernel entry point
		call kernel_main
		# If for whatever reason the kernel exits, disable interrupts and spin forever until a power cycle reset
		cli
		loop:
			hlt
			jmp loop

	.size _start, . - _start
