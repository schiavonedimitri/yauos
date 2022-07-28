#include <stddef.h>
#include <stdint.h>
#include <arch/cpu/cpu.h>
#include <arch/cpu/exception_interrupt.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/io.h>
#include <arch/cpu/smp.h>
#include <kernel/interrupt.h>
#include <kernel/printk.h>
#include <platform/pic.h>

extern bool arch_init;

static interrupt_handler_t interrupt_handler_table[256];

int register_interrupt_handler(uint8_t interrupt_number, interrupt_handler_t handler) {
    
    /* 
     * Return failure in case something tries to register these interrupt numbers:
     * 0 - 31 as they are reserved for CPU exceptions.
     * If the system is not SMP the spurious interrupt numbers of the PIC chip (39 - 47).
     * If the systems is SMP all the PIC interrupt numbers (they could be raised spuriously)
     * and the APIC spurious interrupt vector (255).
     */

    if (interrupt_number <= 31 || (!smp && (interrupt_number == 39 || interrupt_number == 47)) || (smp && !arch_init && (interrupt_number >= 32 || interrupt_number <= 47 || interrupt_number == 255))) {
        return -1;
    }
    interrupt_handler_table[interrupt_number] = handler;
    return 0;
}

void unregister_interrupt_handler(uint8_t interrupt_number) {
    if (interrupt_handler_table[interrupt_number] == NULL) {
        printk("[KERNEL]: Tried to unregister a handler for a non registered interrupt number!\n");
        return;
    }
    else {
        interrupt_handler_table[interrupt_number] = NULL;
    }
}

void interrupt_common_handler(interrupt_context_t *context) {
    if (context->number == 14) {
        panic("Page fault at address: %x\n", read_cr2());
    }
    if (context->number == 39) {
        if (pic_read_register(READ_MASTER | READ_ISR) & ISR_IRQ7_NOT_IN_SERVICE) {
            printk("Spurious interrupt on PIC1 detected!\n");
            return;
        }
        else {
            pic_send_eoi(PIC1_COMMAND_PORT);
        }
    }
    if (context->number == 47) {
        if (pic_read_register(READ_SLAVE | READ_ISR) & ISR_IRQ15_NOT_IN_SERVICE) {
            printk("Spurious interrupt on PIC2 detected!\n");
            pic_send_eoi(PIC1_COMMAND_PORT);
        }
        else {
            pic_send_eoi(PIC1_COMMAND_PORT);
            pic_send_eoi(PIC2_COMMAND_PORT);
        }
    }
    interrupt_handler_t handler = interrupt_handler_table[context->number];
    if (handler != NULL) {
        handler();
    }
    if (context->number >= 40) {
        pic_send_eoi(PIC2_COMMAND_PORT);
    }
    else if (context->number >= 31 && context->number < 40) {
        pic_send_eoi(PIC1_COMMAND_PORT);
    }
}