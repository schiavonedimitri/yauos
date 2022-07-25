#include <arch/cpu/gdt.h>

.extern interrupt_handler
.type interrupt_handler, @function

#define isr_err_stub(num) \
.global isr_err_stub_ ## num ;\
isr_err_stub_ ## num: ;\
pushl $num ;\
jmp isr_common_stub 

#define isr_no_err_stub(num) \
.global isr_no_err_stub_ ## num ;\
isr_no_err_stub_ ## num: ;\
pushl $0 ;\
pushl $num ;\
jmp isr_common_stub

.section .text

isr_no_err_stub(0)
isr_no_err_stub(1)
isr_no_err_stub(2)
isr_no_err_stub(3)
isr_no_err_stub(4)
isr_no_err_stub(5)
isr_no_err_stub(6)
isr_no_err_stub(7)
isr_err_stub(8)
isr_no_err_stub(9)
isr_err_stub(10)
isr_err_stub(11)
isr_err_stub(12)
isr_err_stub(13)
isr_err_stub(14)
isr_no_err_stub(15)
isr_no_err_stub(16)
isr_err_stub(17)
isr_no_err_stub(18)
isr_no_err_stub(19)
isr_no_err_stub(20)
isr_err_stub(21)
isr_no_err_stub(22)
isr_no_err_stub(23)
isr_no_err_stub(24)
isr_no_err_stub(25)
isr_no_err_stub(26)
isr_no_err_stub(27)
isr_no_err_stub(28)
isr_err_stub(29)
isr_err_stub(30)
isr_no_err_stub(31)
isr_no_err_stub(32)
isr_no_err_stub(33)
isr_no_err_stub(34)
isr_no_err_stub(35)
isr_no_err_stub(36)
isr_no_err_stub(37)
isr_no_err_stub(38)
isr_no_err_stub(39)
isr_no_err_stub(40)
isr_no_err_stub(41)
isr_no_err_stub(42)
isr_no_err_stub(43)
isr_no_err_stub(44)
isr_no_err_stub(45)
isr_no_err_stub(46)
isr_no_err_stub(47)

.global isr_common_stub
.type isr_common_stub, @function

isr_common_stub:
    pusha
    mov %ds, %ax
    pushl %eax
    mov $GDT_KERNEL_DATA_OFFSET, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    pushl %esp
    call interrupt_handler
    addl $4, %esp
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    popa
    addl $8, %esp
    iret 
.size isr_common_stub, . - isr_common_stub
