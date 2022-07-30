#ifndef PIC_H
        #define PIC_H

        /*
        * Intel programmable 8259 compatible interrupt controller driver.
        */

        #include <stdint.h>
        #include <arch/cpu/io.h>

        /*
        * On x86 pc-at compatible computers there's usually 2 pics in cascade that can serve up to 15
        * IRQs (one of the lines is used for inter-pic signaling):
        */

        #define PIC1_COMMAND_PORT 0x20
        #define PIC1_DATA_PORT 0x21
        #define PIC2_COMMAND_PORT 0xA0
        #define PIC2_DATA_PORT 0xA1

        /*
        * As per Intel manual the PIC initialization protocol states to send ICW1 ICW2 ICW3 and optionally depending on ICW1 ICW4.
        * After booting on x86 usually the BIOS or the firmware configures the PICS to serve interrupts 0 - 7 with interrupt numbers
        * 0x08 - 0x0F (Master) and interrupts 8 - 15 with interrupt numbers 0x70 - 0x77 (Slave).
        * In protected mode on x86 architecture the CPU reserved the first 31 interrupt numbers for its own use and as such a PIC
        * remapping is needed. In order to remap the pics a full initialization sequence must be sent.
        */

        /*
        * ICW1 defnitions:
        * As per Intel manual this initialization word is used as part of the initialization protocol. For x86 
        * the only relevant bits set the interrupt trigger mode which can be level or edge triggered. The ICW4
        * needed bit is used to specify one more word to specify the EOI mode and finally the bit that
        * tells if the PIC works in single mode or cascade mode with a slave (which is the case on x86).
        * This word must be sent to the PIC command port. If the cascade mode bit is set and if there is more than
        * one PIC (such as on x86) this word must be sent to both the master and slave PIC on the respective
        * command port.
        */

        #define ICW1_ICW4_NEEDED 0x1
        #define ICW1_ICW4_NOT_NEEDED 0x0
        #define ICW1_SINGLE 0x2
        #define ICW1_CASCADE 0x0
        #define ICW1_LEVEL_TRIGGERED 0x8
        #define ICW1_EDGE_TRIGGERED 0x0
        #define ICW1_INIT 0x10

        /*
        * ICW2 definitions:
        * As per Intel manual this word must be sent after ICW1 and specifies the vector offset for the PIC irqs.
        * This word is used for remapping the interrupt numbers that a PIC sends to the CPU for any IRQ line.
        * If more than one PIC is present, this word must be sent to both PICS on the respective data ports.
        */

        #define ICW2(offset) offset & 0xF8

        /*
        * ICW3 definitions:
        * As per Intel manual this word must come after ICW2 if and only if the cascade mode bit is set.
        * This word is used to tell the master and slave PICS which IRQ line to use for communication 
        * between themselves. 
        */

        #define ICW3_MASTER_SLAVE_AT_IRQ0 0x0
        #define ICW3_MASTER_SLAVE_AT_IRQ1 0x2
        #define ICW3_MASTER_SLAVE_AT_IRQ2 0x4
        #define ICW3_MASTER_SLAVE_AT_IRQ3 0x8
        #define ICW3_MASTER_SLAVE_AT_IRQ4 0x10
        #define ICW3_MASTER_SLAVE_AT_IRQ5 0x20
        #define ICW3_MASTER_SLAVE_AT_IRQ6 0x40
        #define ICW3_MASTER_SLAVE_AT_IRQ7 0x80
        #define ICW3_SLAVE_MASTER_AT_IRQ0 0x0
        #define ICW3_SLAVE_MASTER_AT_IRQ1 0x1
        #define ICW3_SLAVE_MASTER_AT_IRQ2 0x2
        #define ICW3_SLAVE_MASTER_AT_IRQ3 0x3
        #define ICW3_SLAVE_MASTER_AT_IRQ4 0x4
        #define ICW3_SLAVE_MASTER_AT_IRQ5 0x5
        #define ICW3_SLAVE_MASTER_AT_IRQ6 0x6
        #define ICW3_SLAVE_MASTER_AT_IRQ7 0x7

        /*
        * ICW4 definitions:
        * As per Intel manual this word must come after ICW2 or ICW3 if the cascade mode bit was set in ICW1.
        * This word is usded to to specify the interrupt mode and the EOI mode.
        */

        #define ICW4_80_86_MODE 0x1
        #define ICW4_AUTO_EOI 0x2
        #define ICW4_NO_AUTO_EOI 0x0
        #define ICW4_BUFFERED_MODE_MASTER 0xC
        #define ICW4_BUFFERED_MODE_SLAVE 0x8
        #define ICW4_NOT_BUFFERED_MODE 0x0
        #define ICW4_SFNM_MODE 0x10
        #define ICW4_SEQUENTIAL_MODE 0x0

        /* 
        * OCW are words used after initialization is complete and can come in any order.
        */

        /*
        * OCW1 definitions:
        * As per Intel manual OCW1 must be sent to the PICS data ports.
        * The effect is to enable or disable a specific iRQ line specified by
        * the corresponding bit. If the bit is set the line is enabled and disabled
        * otherwise.
        */

        #define OCW1_ENABLE_IRQ0 0x0
        #define OCW1_DISABLE_IRQ0 0x1
        #define OCW1_ENABLE_IRQ1 0x0
        #define OCW1_DISABLE_IRQ1 0x2
        #define OCW1_ENABLE_IRQ2 0x0
        #define OCW1_DISABLE_IRQ2 0x4
        #define OCW1_ENABLE_IRQ3 0x0
        #define OCW1_DISABLE_IRQ3 0x8
        #define OCW1_ENABLE_IRQ4 0x0
        #define OCW1_DISABLE_IRQ4 0x10
        #define OCW1_ENABLE_IRQ5 0x0
        #define OCW1_DISABLE_IRQ5 0x20
        #define OCW1_ENABLE_IRQ6 0x0
        #define OCW1_DISABLE_IRQ6 0x40
        #define OCW1_ENABLE_IRQ7 0x0
        #define OCW1_DISABLE_IRQ7 0x80
        #define OCW1_ENABLE_ALL_IRQS 0x0
        #define OCW1_DISABLE_ALL_IRQS 0xFF

        /* 
        * OCW2 definitions:
        * As per Intel manual OCW2 must be sent to the PICS command ports.
        * Bits 0 - 2 determine the interrupt request level to act upon when bit 6 is set.
        * The same manual also states that bits 0 - 2 are used  only for the 
        * "set priority command" and for the "rotate on specific eoi command".
        * This word is used to set the EOI mode and to specify the priority level
        * of any IRQ line.
        */

        #define OCW2_INTERRUPT_REQUEST_LEVEL0 0x0
        #define OCW2_INTERRUPT_REQUEST_LEVEL1 0x1
        #define OCW2_INTERRUPT_REQUEST_LEVEL2 0x2
        #define OCW2_INTERRUPT_REQUEST_LEVEL3 0x3
        #define OCW2_INTERRUPT_REQUEST_LEVEL4 0x4
        #define OCW2_INTERRUPT_REQUEST_LEVEL5 0x5
        #define OCW2_INTERRUPT_REQUEST_LEVEL6 0x6
        #define OCW2_INTERRUPT_REQUEST_LEVEL7 0x7
        #define OCW2_EOI_ROTATE_AUTOMATIC_CLEAR 0x0
        #define OCW2_EOI_NON_SPECIFIC_COMMAND 0x20
        #define OCW2_EOI_NO_OPERATION 0x40
        #define OCW2_EOI_SPECIFIC_COMMAND 0x60
        #define OCW2_EOI_ROTATE_AUTOMATIC_SET 0x80
        #define OCW2_EOI_ROTATE_NON_SPECIFIC_COMMAND 0xA0
        #define OCW2_EOI_SET_PRIORITY_COMMAND 0xC0
        #define OCW2_EOI_ROTATE_ON_SPECIFIC_COMMAND 0xE0

        /*
        * OCW3 definitions:
        * As per Intel manual OCW3 must be sent to the PICS command ports.
        * This word is used to read from the IRR or ISR register, to set or reset
        * the special mask mode orto issue a poll command.
        * When reading the IRR or ISR registers the OCW3 must be sent to the relevant PIC
        * command port and then read on the same port to get the register contents.
        * If reading multiple times without issuing a new and different OCW3 the manual
        * states that there's no need to send OCW3 again (that is, it is saved in an internal buffer of the PiC).
        * It is sufficient to read from the relevant PIC command port as many times as need as long as a new OCW3
        * is not issued.
        */

        #define OCW3_READ_IRR 0x0
        #define OCW3_READ_ISR 0x1
        #define OCW3_READ_COMMAND 0x2
        #define OCW3_IGNORE_READ_COMMAND 0x0
        #define OCW3_POLL_COMMAND 0x4
        #define OCW3_IGNORE_POLL_COMMAND 0x0
        #define OCW3_COMMAND 0x8
        #define OCW3_SET_SPECIAL_MASK 0x20
        #define OCW3_RESET_SPECIAL_MASK 0x0
        #define OCW3_SPECIAL_MASK_COMMAND 0x40
        #define OCW3_IGNORE_SPECIAL_MASK_COMMAND 0x0

        /*
        * Definitions for pic_read_register();
        */

        #define READ_IRR 0x0
        #define READ_ISR 0x1
        #define READ_MASTER 0x0
        #define READ_SLAVE 0x2

        /* 
        * Definitions for IRR register read values for PIC 1.
        */

        #define IRR_IRQ0_RAISED 0x1
        #define IRR_IRQ0_NOT_RAISED 0x0
        #define IRR_IRQ1_RAISED 0x2
        #define IRR_IRQ1_NOT_RAISED 0x0
        #define IRR_IRQ2_RAISED 0x4
        #define IRR_IRQ2_NOT_RAISED 0x0
        #define IRR_IRQ3_RAISED 0x8
        #define IRR_IRQ3_NOT_RAISED 0x0
        #define IRR_IRQ4_RAISED 0x10
        #define IRR_IRQ4_NOT_RAISED 0x0
        #define IRR_IRQ5_RAISED 0x20
        #define IRR_IRQ5_NOT_RAISED 0x0
        #define IRR_IRQ6_RAISED 0x40
        #define IRR_IRQ6_NOT_RAISED 0x0
        #define IRR_IRQ7_RAISED 0x80
        #define IRR_IRQ7_NOT_RAISED 0x0

        /* 
        * Definitions for IRR register read values for PIC 2.
        */

        #define IRR_IRQ8_RAISED 0x1
        #define IRR_IRQ8_NOT_RAISED 0x0
        #define IRR_IRQ9_RAISED 0x2
        #define IRR_IRQ9_NOT_RAISED 0x0
        #define IRR_IRQ10_RAISED 0x4
        #define IRR_IRQ10_NOT_RAISED 0x0
        #define IRR_IRQ11_RAISED 0x8
        #define IRR_IRQ11_NOT_RAISED 0x0
        #define IRR_IRQ12_RAISED 0x10
        #define IRR_IRQ12_NOT_RAISED 0x0
        #define IRR_IRQ13_RAISED 0x20
        #define IRR_IRQ13_NOT_RAISED 0x0
        #define IRR_IRQ14_RAISED 0x40
        #define IRR_IRQ14_NOT_RAISED 0x0
        #define IRR_IRQ15_RAISED 0x80
        #define IRR_IRQ15_NOT_RAISED 0x0

        /* 
        * Definitions for ISR register read values for PIC 1.
        */

        #define ISR_IRQ0_IN_SERVICE 0x1
        #define ISR_IRQ0_NOT_IN_SERVICE 0x0
        #define ISR_IRQ1_IN_SERVICE 0x2
        #define ISR_IRQ1_NOT_IN_SERVICE 0x0
        #define ISR_IRQ2_IN_SERVICE 0x4
        #define ISR_IRQ2_NOT_IN_SERVICE 0x0
        #define ISR_IRQ3_IN_SERVICE 0x8
        #define ISR_IRQ3_NOT_IN_SERVICE 0x0
        #define ISR_IRQ4_IN_SERVICE 0x10
        #define ISR_IRQ4_NOT_IN_SERVICE 0x0
        #define ISR_IRQ5_IN_SERVICE 0x20
        #define ISR_IRQ5_NOT_IN_SERVICE 0x0
        #define ISR_IRQ6_IN_SERVICE 0x40
        #define ISR_IRQ6_NOT_IN_SERVICE 0x0
        #define ISR_IRQ7_IN_SERVICE 0x80
        #define ISR_IRQ7_NOT_IN_SERVICE 0x0

        /* 
        * Definitions for ISR register read values for PIC 2.
        */

        #define ISR_IRQ8_IN_SERVICE 0x1
        #define ISR_IRQ8_NOT_IN_SERVICE 0x0
        #define ISR_IRQ9_IN_SERVICE 0x2
        #define ISR_IRQ9_NOT_IN_SERVICE 0x0
        #define ISR_IRQ10_IN_SERVICE 0x4
        #define ISR_IRQ10_NOT_IN_SERVICE 0x0
        #define ISR_IRQ11_IN_SERVICE 0x8
        #define ISR_IRQ11_NOT_IN_SERVICE 0x0
        #define ISR_IRQ12_IN_SERVICE 0x10
        #define ISR_IRQ12_NOT_IN_SERVICE 0x0
        #define ISR_IRQ13_IN_SERVICE 0x20
        #define ISR_IRQ13_NOT_IN_SERVICE 0x0
        #define ISR_IRQ14_IN_SERVICE 0x40
        #define ISR_IRQ14_NOT_IN_SERVICE 0x0
        #define ISR_IRQ15_IN_SERVICE 0x80
        #define ISR_IRQ15_NOT_IN_SERVICE 0x0

        void pic_init(void);
        void pic_restore_real_mode_configuration(void);
        void pic_enable_all_irq_lines(void);
        void pic_enable_irq_line(uint8_t);
        void pic_disable_all_irq_lines(void);
        void pic_disable_irq_line(uint8_t);
        uint8_t pic_read_register(uint8_t);

        inline static void pic_send_eoi(uint8_t pic) {
                outb(pic, OCW2_EOI_NON_SPECIFIC_COMMAND);    
        }

#endif /** PIC_H */