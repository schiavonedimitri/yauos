.section .text
	.global arch_halt
	.type arch_halt, @function
	arch_halt:
		cli
                _loop:
                        hlt
                        jmp _loop
	.size arch_halt, . - arch_halt