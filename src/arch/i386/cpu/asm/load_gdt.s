.section .text
	.global load_gdt
	.type load_gdt, @function

	load_gdt:
		movl 4(%esp), %eax
		lgdt (%eax)
		movl $0x10, %eax
		movl %eax, %ds
		movl %eax, %es
		movl %eax, %fs
		movl %eax, %gs
		movl %eax, %ss
		ljmp $0x8, $return

	return:
	ret
	.size load_gdt, . - load_gdt
