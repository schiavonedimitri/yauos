#include <stdint.h>
#include <arch/cpu/io.h>
#include <platform/pit.h>

void pit_interrupt_on_terminal_count(uint16_t count) {
    outb(PIT_COMMAND_MODE_REGISTER, PIT_COMMAND_CHANNEL_0 | PIT_COMMAND_ACCESS_LOW_BYTE_HIGH_BYTE | PIT_COMMAND_MODE_INTERRUPT_ON_TERMINAL_COUNT | PIT_COMMAND_BINARY_MODE);
    outb(PIT_CHANNEL_0_DATA_REGISTER, (uint8_t) count);
    outb(PIT_CHANNEL_0_DATA_REGISTER, (uint8_t) ((count >> 8) & 0xFF));
}

void pit_interrupt_rate_generator(uint16_t frequency) {
    uint16_t frequency_divisor = 1193180 / frequency;
    outb(PIT_COMMAND_MODE_REGISTER, PIT_COMMAND_CHANNEL_0 | PIT_COMMAND_ACCESS_LOW_BYTE_HIGH_BYTE | PIT_COMMAND_MODE_RATE_GENERATOR | PIT_COMMAND_BINARY_MODE);
    outb(PIT_CHANNEL_0_DATA_REGISTER, (uint8_t) frequency_divisor);
    outb(PIT_CHANNEL_0_DATA_REGISTER, (uint8_t) ((frequency_divisor >> 8) & 0xFF));
}

uint16_t pit_get_counter_value(uint8_t channel) {
    uint16_t counter_value = -1;
    switch(channel) {
        case 0:
            outb(PIT_COMMAND_MODE_REGISTER, PIT_COMMAND_READ_BACK | PIT_COMMAND_READ_BACK_LATCH_COUNT | PIT_COMMAND_READ_BACK_DONT_LATCH_STATUS | PIT_COMMAND_READ_BACK_CHANNEL_0);
            counter_value = inb(PIT_CHANNEL_0_DATA_REGISTER);
            counter_value |= inb(PIT_CHANNEL_0_DATA_REGISTER) << 8;
            return counter_value;
        case 1:
            outb(PIT_COMMAND_MODE_REGISTER, PIT_COMMAND_READ_BACK | PIT_COMMAND_READ_BACK_LATCH_COUNT | PIT_COMMAND_READ_BACK_DONT_LATCH_STATUS | PIT_COMMAND_READ_BACK_CHANNEL_1);
            counter_value = inb(PIT_CHANNEL_1_DATA_REGISTER);
            counter_value |= inb(PIT_CHANNEL_1_DATA_REGISTER) << 8;
            return counter_value;
        case 2:
            outb(PIT_COMMAND_MODE_REGISTER, PIT_COMMAND_READ_BACK | PIT_COMMAND_READ_BACK_LATCH_COUNT | PIT_COMMAND_READ_BACK_DONT_LATCH_STATUS | PIT_COMMAND_READ_BACK_CHANNEL_2);
            counter_value = inb(PIT_CHANNEL_2_DATA_REGISTER);
            counter_value |= inb(PIT_CHANNEL_2_DATA_REGISTER) << 8;
            return counter_value;
        default:
            return counter_value;            
    }
}