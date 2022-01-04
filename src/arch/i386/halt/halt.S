.section .text
	.global halt
	.type halt, @function
	halt:
		cli
	_loop:
		hlt
		jmp _loop

	.size halt, . - halt
