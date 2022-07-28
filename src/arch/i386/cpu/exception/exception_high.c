#include <stddef.h>
#include <stdint.h>
#include <arch/cpu/cpu.h>
#include <arch/cpu/exception_interrupt.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/io.h>
#include <arch/cpu/smp.h>
#include <kernel/printk.h>

void exception_common_handler(exception_context_t *context) {
    switch(context->number) {
        default:
            if (context->cs == GDT_USER_CODE_OFFSET) {
                // TODO: when user space is implemented, kill or act accordingly for the process causing this exception.
            }
            else {
                panic("[KERNEL]: CPU[%x]: Exception number %d occurred while running kernel code!\n", cpu->lapic_id, context->number);
            }
    }
}