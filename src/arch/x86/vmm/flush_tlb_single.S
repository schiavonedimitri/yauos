.section .text
	.global flush_tlb_single
	.type flush_tlb_single, @function
	flush_tlb_single:
		movl 4(%esp), %eax
		invlpg (%eax)
		ret
	.size flush_tlb_single, . - flush_tlb_single
