#include <stddef.h>
#include <stdint.h>
#include <arch/cpu/cpu.h>
#include <arch/cpu/exception_interrupt.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/io.h>
#include <arch/cpu/smp.h>
#include <arch/kernel/mm/vm.h>
#include <kernel/printk.h>

void exception_common_handler(exception_context_t *context) {
    switch(context->number) {
        case 14:
            do_page_fault(read_cr2());
            break;
        default:
            if (context->cs == GDT_USER_CODE_OFFSET) {
                // TODO: when user space is implemented, kill or act accordingly for the process causing this exception.
            }
            else {
                panic("[KERNEL]: Exception number %x with error code: %x occurred while running kernel code on CPU %x\nRegister dump:\nEAX = %x\nEBX = %x\nECX = %x\nEDX = %x\nESI = %x\nEDI = %x\nEBP = %x\nEIP = %x\nCS = %x\nSS = %x\nDS = %x\nES = %x\nFS = %x\nGS = %x\nEFLAGS = %x\n", context->number, context->error_code, cpu->lapic_id, context->eax, context->ebx, context->ecx, context->edx, context->esi, context->edi, context->ebp, context->eip, context->cs, context->kernel_ss, context->ds, context->es, context->fs, context->gs, context->eflags);
            }
    }
}