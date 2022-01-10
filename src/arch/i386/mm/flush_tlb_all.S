 .section .text
	.global flush_tlb_all
	.type flush_tlb_all, @function
	flush_tlb_all:
		movl %cr3, %eax
		movl %eax, %cr3
		ret
	.size flush_tlb_all, . - flush_tlb_all
