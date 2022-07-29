#include <stddef.h>
#include <stdint.h>
#include <arch/cpu/cpu.h>
#include <arch/cpu/exception_interrupt.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/io.h>
#include <arch/cpu/smp.h>
#include <arch/kernel/mm/vm.h>
#include <lib/string.h>
#include <kernel/printk.h>

/*
 * Defined in stack_trace.S
 * TODO: implement loading kernel symbols and function name lookups for stack traces.
 */

extern uint32_t walk_stack(uint32_t *array, uint32_t number_frames);

void exception_common_handler(exception_context_t *context) {
    static uint32_t nested_counter = 0;

    /*
     * The following code could happen if the call chain beginning here ends up 
     * causing cascading exceptions (should not because kernel code must be robust, but kernel bugs happen anyway!)
     * if more than 3 nested exceptions happen print the last exception data and trace and give up executing!
     * TODO: signal other cpus to hang up aswell. 
     */

    if (++nested_counter >= 3) {
        uint32_t stack_trace[10];
        memset(stack_trace, 0x0, sizeof(uint32_t) * 10);
        uint32_t n_frames = walk_stack(stack_trace, 10);
        printk("[KERNEL]: Nested exception loop detected. Last exception information:\n[KERNEL]: Exception number %x with error code: %x occurred while running kernel code on CPU %x\nRegister dump:\nEAX = %x\nEBX = %x\nECX = %x\nEDX = %x\nESI = %x\nEDI = %x\nEBP = %x\nEIP = %x\nCS = %x\nSS = %x\nDS = %x\nES = %x\nFS = %x\nGS = %x\nEFLAGS = %x\nBack trace:\n", context->number, context->error_code, cpu->lapic_id, context->eax, context->ebx, context->ecx, context->edx, context->esi, context->edi, context->ebp, context->eip, context->cs, context->kernel_ss, context->ds, context->es, context->fs, context->gs, context->eflags);
        for (size_t i = 0; i < n_frames; i++) {
            printk("[%x]: %x\n", i, stack_trace[i]);
        }
        panic("[KERNEL]: End of trace.");
    }
    switch(context->number) {
        case 14:
            do_page_fault(read_cr2());
            break;
        default:
            if (context->cs == GDT_USER_CODE_OFFSET) {
                // TODO: when user space is implemented send the appropriate signal to the offending process.
            }
            else {
                uint32_t stack_trace[10];
                memset(stack_trace, 0x0, sizeof(uint32_t) * 10);
                uint32_t n_frames = walk_stack(stack_trace, 10);
                printk("[KERNEL]: Exception number %x with error code: %x occurred while running kernel code on CPU %x\nRegister dump:\nEAX = %x\nEBX = %x\nECX = %x\nEDX = %x\nESI = %x\nEDI = %x\nEBP = %x\nEIP = %x\nCS = %x\nSS = %x\nDS = %x\nES = %x\nFS = %x\nGS = %x\nEFLAGS = %x\nBack trace:\n", context->number, context->error_code, cpu->lapic_id, context->eax, context->ebx, context->ecx, context->edx, context->esi, context->edi, context->ebp, context->eip, context->cs, context->kernel_ss, context->ds, context->es, context->fs, context->gs, context->eflags);
                for (size_t i = 0; i < n_frames; i++) {
                    printk("[%x]: %x\n", i, stack_trace[i]);
                }
                panic("[KERNEL]: End of trace.");
            }
    }
    nested_counter--;
}