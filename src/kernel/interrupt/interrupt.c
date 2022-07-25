#include <stddef.h>
#include <stdint.h>
#include <arch/cpu/cpu.h>
#include <arch/cpu/io.h>
#include <arch/kernel/interrupt.h>
#include <kernel/interrupt.h>
#include <kernel/printk.h>
#include <platform/pic.h>

static interrupt_handler_t interrupt_handler_table[256];

void interrupt_handler(interrupt_context_t *context) {
    if (context->interrupt_number == 14) {
        panic("Page fault at address: %x\n", read_cr2());
    }
    if (context->interrupt_number == 39) {
        if (pic_read_register(READ_MASTER | READ_ISR) & ISR_IRQ7_NOT_IN_SERVICE) {
            printk("Spurious interrupt on PIC1 detected!\n");
            return;
        }
        else {
            pic_send_eoi(PIC1_COMMAND_PORT);
        }
    }
    if (context->interrupt_number == 47) {
        if (pic_read_register(READ_SLAVE | READ_ISR) & ISR_IRQ15_NOT_IN_SERVICE) {
            printk("Spurious interrupt on PIC2 detected!\n");
            pic_send_eoi(PIC1_COMMAND_PORT);
        }
        else {
            pic_send_eoi(PIC1_COMMAND_PORT);
            pic_send_eoi(PIC2_COMMAND_PORT);
        }
    }
    interrupt_handler_t handler = interrupt_handler_table[context->interrupt_number];
    if (handler != NULL) {
        handler(context);
    }
    if (context->interrupt_number >= 40) {
        pic_send_eoi(PIC2_COMMAND_PORT);
    }
    else if (context->interrupt_number >= 31 && context->interrupt_number < 40) {
        pic_send_eoi(PIC1_COMMAND_PORT);
    }
}

int register_interrupt_handler(uint8_t interrupt_number, interrupt_handler_t handler) {
    if (interrupt_number <= 31) {
        return -1;
    }
    interrupt_handler_table[interrupt_number] = handler;
    return 0;
}