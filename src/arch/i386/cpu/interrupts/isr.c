#include <stdint.h>
#include <kernel/printk.h>
extern void arch_halt();

typedef struct registers
{
   uint32_t ds;
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
   uint32_t int_no, err_code;
   uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

void interrupt_handler(registers_t regs) {
    printk("Interrupt: %x error code: %x\n", regs.int_no, regs.err_code);
    if (regs.int_no == 0xE) {
        uint32_t faulting_address;
        asm volatile("movl %%cr2, %0" : "=r" (faulting_address));
        printk("Faulting address: %x\nInstruction pointer: %x\n", faulting_address, regs.eip);
    }
}