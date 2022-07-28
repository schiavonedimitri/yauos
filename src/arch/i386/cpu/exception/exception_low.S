#include <arch/cpu/gdt.h>

.extern exception_common_handler
.type exception_common_handler, @function

#define exception_err_stub(num) \
.global exception_err_stub_ ## num ;\
exception_err_stub_ ## num: ;\
pushl $num ;\
jmp exception_common_stub

#define exception_no_err_stub(num) \
.global exception_no_err_stub_ ## num ;\
exception_no_err_stub_ ## num: ;\
pushl $0 ;\
pushl $num ;\
jmp exception_common_stub

.section .text

# Exception stubs.

exception_no_err_stub(0)
exception_no_err_stub(1)
exception_no_err_stub(2)
exception_no_err_stub(3)
exception_no_err_stub(4)
exception_no_err_stub(5)
exception_no_err_stub(6)
exception_no_err_stub(7)
exception_err_stub(8)
exception_no_err_stub(9)
exception_err_stub(10)
exception_err_stub(11)
exception_err_stub(12)
exception_err_stub(13)
exception_err_stub(14)
exception_no_err_stub(15)
exception_no_err_stub(16)
exception_err_stub(17)
exception_no_err_stub(18)
exception_no_err_stub(19)
exception_no_err_stub(20)
exception_err_stub(21)
exception_no_err_stub(22)
exception_no_err_stub(23)
exception_no_err_stub(24)
exception_no_err_stub(25)
exception_no_err_stub(26)
exception_no_err_stub(27)
exception_no_err_stub(28)
exception_err_stub(29)
exception_err_stub(30)
exception_no_err_stub(31)

.global exception_common_stub
.type exception_common_stub, @function

exception_common_stub:
    pusha
    mov %ds, %ax
    pushl %eax
    mov $GDT_KERNEL_DATA_OFFSET, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov $GDT_KERNEL_PER_CPU_DATA_OFFSET, %ax
    mov %ax, %gs
    pushl %esp
    call exception_common_handler
    addl $4, %esp
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs

    # TODO: for now interrupts start and return to kernel so reload kernel gs segment.
    # When user mode will be implemented remember this and act accordingly! 

    mov $GDT_KERNEL_PER_CPU_DATA_OFFSET, %ax
    mov %ax, %gs
    popa
    addl $8, %esp
    iret 
.size exception_common_stub, . - exception_common_stub