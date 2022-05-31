#include <arch/cpu/io.h>
#include <platform/pic.h>

static uint8_t master_real_mode_interrupt_mask = 0;
static uint8_t slave_real_mode_interrupt_mask = 0;
static uint8_t last_ocw3_master = 0;
static uint8_t last_ocw3_slave = 0;

/*
 * Intel 8259 pic family initialization code:
 * 1) Send ICW1_INIT to both pic command ports specifying other settings for ICW1 if needed.
 * 2) On x86 the pics are initialized for suitable use in real mode but are not suited for operation
 * in protected mode. That's because irqs 0 - 7 are mapped to interrupt numbers 8 - 15 but in protected
 * mode interrupt numbers 0 - 31 are reserved for the CPU so a remapping of the pic vectors is needed.
 * ICW2 does that and specifies which vector offsets a given pic will use by writing ICW2 to the pics data port.
 * 3) If ICW1 specified the use of multiple pics in cascade mode ICW3 must be sent to both master and 
 * slave pics to tell them which irq lines to use for communication between them by writing to the pics data port.
 * 4) If ICW1 specified the need for ICW4, ICW4 must be sent to the pic or pics data port.   
 */

// Note: io_wait() calls are put just for ancient hardware which might not be fast enough to complete io writes.

void pic_init() {
    // Save old interrupt masks to be able to return to real mode configuration.
    master_real_mode_interrupt_mask = inb(PIC1_DATA_PORT);
    slave_real_mode_interrupt_mask = inb(PIC2_DATA_PORT);
    // Send ICW1 to master pic.
    outb(PIC1_COMMAND_PORT, ICW1_INIT | ICW1_ICW4_NEEDED | ICW1_CASCADE | ICW1_EDGE_TRIGGERED);
    io_wait();
    // Send ICW1 to slave pic.
    outb(PIC2_COMMAND_PORT, ICW1_INIT | ICW1_ICW4_NEEDED | ICW1_CASCADE | ICW1_EDGE_TRIGGERED);
    io_wait();
    // Remap master pic irqs to interrupt numbers 32 - 39.
    outb(PIC1_DATA_PORT, ICW2(0x20));
    io_wait();
    // Remap slave pic irqs to interrupt numbers 40 - 47.
    outb(PIC2_DATA_PORT, ICW2(0x28));
    io_wait();
    // Tell master there's a salve at IRQ2 line.
    outb(PIC1_DATA_PORT, ICW3_MASTER_SLAVE_AT_IRQ2);
    io_wait();
    // Tell slave there's a master at IRQ2 line.
    outb(PIC2_DATA_PORT, ICW3_SLAVE_MASTER_AT_IRQ2);
    io_wait();
    // Set master pic in 80x86 mode with no auto eoi and sequential mode.
    outb(PIC1_DATA_PORT, ICW4_80_86_MODE | ICW4_NO_AUTO_EOI | ICW4_SEQUENTIAL_MODE);
    io_wait();
    // Set slave pic in 80x86 mode with no auto eoi and sequential mode.
    outb(PIC2_DATA_PORT, ICW4_80_86_MODE | ICW4_NO_AUTO_EOI | ICW4_SEQUENTIAL_MODE);
    io_wait();
    // Disable all irq lines initially.
    outb(PIC1_DATA_PORT, OCW1_DISABLE_ALL_IRQS);
    io_wait();
    outb(PIC2_DATA_PORT, OCW1_DISABLE_ALL_IRQS);
    io_wait();
}

void pic_restore_real_mode_configuration() {
    // Send ICW1 to master pic.
    outb(PIC1_COMMAND_PORT, ICW1_INIT | ICW1_ICW4_NEEDED | ICW1_CASCADE | ICW1_EDGE_TRIGGERED);
    io_wait();
    // Send ICW1 to slave pic.
    outb(PIC2_COMMAND_PORT, ICW1_INIT | ICW1_ICW4_NEEDED | ICW1_CASCADE | ICW1_EDGE_TRIGGERED);
    io_wait();
    // Restore master pic irqs.
    outb(PIC1_DATA_PORT, ICW2(0x08));
    io_wait();
    // Restore slave pic irqs.
    outb(PIC2_DATA_PORT, ICW2(0x70));
    io_wait();
    // Tell master there's a salve at IRQ2 line.
    outb(PIC1_DATA_PORT, ICW3_MASTER_SLAVE_AT_IRQ2);
    io_wait();
    // Tell slave there's a master at IRQ2 line.
    outb(PIC2_DATA_PORT, ICW3_SLAVE_MASTER_AT_IRQ2);
    io_wait();
    // Set master pic in 80x86 mode with no auto eoi and sequential mode.
    outb(PIC1_DATA_PORT, ICW4_80_86_MODE | ICW4_NO_AUTO_EOI | ICW4_SEQUENTIAL_MODE);
    io_wait();
    // Set slave pic in 80x86 mode with no auto eoi and sequential mode.
    outb(PIC2_DATA_PORT, ICW4_80_86_MODE | ICW4_NO_AUTO_EOI | ICW4_SEQUENTIAL_MODE);
    io_wait();
    // Restore master interrupt mask.
    outb(PIC1_DATA_PORT, master_real_mode_interrupt_mask);
    io_wait();
    // Retore slave interrupt mask.
    outb(PIC2_DATA_PORT, slave_real_mode_interrupt_mask);
    io_wait();
}

void pic_enable_all_irq_lines() {
    outb(PIC1_DATA_PORT, OCW1_ENABLE_ALL_IRQS);
    io_wait();
    outb(PIC2_DATA_PORT, OCW1_ENABLE_ALL_IRQS);
    io_wait();
}

void pic_enable_irq_line(uint8_t irq_line) {
    if (irq_line < 8) {
        outb(PIC1_DATA_PORT, inb(PIC1_DATA_PORT) & ~(0x1 << irq_line));
        io_wait();
    }
    else {
        outb(PIC2_DATA_PORT, inb(PIC2_DATA_PORT) & ~(0x0 << irq_line));
        io_wait();
    }
}

void pic_disable_all_irq_lines() {
    outb(PIC1_DATA_PORT, OCW1_DISABLE_ALL_IRQS);
    io_wait();
    outb(PIC2_DATA_PORT, OCW1_DISABLE_ALL_IRQS);
    io_wait();
}

void pic_disable_irq_line(uint8_t irq_line) {
    if (irq_line < 8) {
        outb(PIC1_DATA_PORT, inb(PIC1_DATA_PORT) | (0x1 << irq_line));
        io_wait();
    }
    else {
        outb(PIC2_DATA_PORT, inb(PIC2_DATA_PORT) | (0x1 << irq_line));
        io_wait();
    }
}

/*
 * This function is used to read from a pic register. The pic and ther register are specified
 * in the flags bitfield. Refer to the header file for proper definitions to use when invoking this
 * function.
 * bits: 1 0
 *       0 0 --> read IRR from master pic.
 *       0 1 --> read ISR from master pic.
 *       1 0 --> read IRR from slave pic.
 *       1 1 --> read ISR from slave pic.
 */

uint8_t pic_read_register(uint8_t flags) {
    uint8_t ocw3 = OCW3_READ_COMMAND | OCW3_IGNORE_POLL_COMMAND | OCW3_COMMAND | OCW3_IGNORE_SPECIAL_MASK_COMMAND;
    if (flags & 0x1) {
        ocw3 |= OCW3_READ_ISR;
    }
    else {
        ocw3 |= OCW3_READ_IRR;
    }
    if ((flags >> 1) & 0x1) {
        if (ocw3 == last_ocw3_slave) {
            return inb(PIC2_COMMAND_PORT);
        }
        else {
            outb(PIC2_COMMAND_PORT, ocw3);
            io_wait();
            last_ocw3_slave = ocw3;
            return inb(PIC2_COMMAND_PORT);
        }
    }
    else {
        if (ocw3 == last_ocw3_master) {
            return inb(PIC1_COMMAND_PORT);
        }
        else {
            outb(PIC1_COMMAND_PORT, ocw3);
            io_wait();
            last_ocw3_master = ocw3;
            return inb(PIC1_COMMAND_PORT);
        }
    }
}