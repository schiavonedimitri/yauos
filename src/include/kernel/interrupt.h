#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>
#include <arch/kernel/interrupt.h>

typedef void (*interrupt_handler_t)(interrupt_context_t *context);

int register_interrupt_handler(uint8_t, interrupt_handler_t);

#endif /** INTERRUPT_H */