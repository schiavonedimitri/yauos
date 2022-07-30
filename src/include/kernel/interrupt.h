#ifndef INTERRUPT_H
        #define INTERRUPT_H

        #include <stdint.h>
        #include <arch/cpu/exception_interrupt.h>

        typedef void (*interrupt_handler_t)(void);

        int register_interrupt_handler(uint8_t, interrupt_handler_t);
        void unregister_interrupt_handler(uint8_t);

#endif /** INTERRUPT_H */