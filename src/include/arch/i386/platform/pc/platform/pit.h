#ifndef PIT_H
        #define PIT_H

        #include <stdint.h>

        #define PIT_CHANNEL_0_DATA_REGISTER 0x40
        #define PIT_CHANNEL_1_DATA_REGISTER 0x41
        #define PIT_CHANNEL_2_DATA_REGISTER 0x42
        #define PIT_COMMAND_MODE_REGISTER 0x43

        #define PIT_COMMAND_CHANNEL_0 0x0
        #define PIT_COMMAND_CHANNEL_1 0x80
        #define PIT_COMMAND_CHANNEL_2 0x40
        #define PIT_COMMAND_READ_BACK 0xC0
        #define PIT_COMMAND_LATCH_COUNT 0x0
        #define PIT_COMMAND_ACCESS_LOW_BYTE_ONLY 0x20
        #define PIT_COMMAND_ACCESS_HIGH_BYTE_ONLY 0x10
        #define PIT_COMMAND_ACCESS_LOW_BYTE_HIGH_BYTE 0x30
        #define PIT_COMMAND_MODE_INTERRUPT_ON_TERMINAL_COUNT 0x0
        #define PIT_COMMAND_MODE_ONE_SHOT 0x8
        #define PIT_COMMAND_MODE_RATE_GENERATOR 0x4
        #define PIT_COMMAND_MODE_SQUARE_WAVE_GENERATOR 0xC
        #define PIT_COMMAND_MODE_SOFTWARE_TRIGGERED_STROBE 0x2
        #define PIT_COMMAND_MODE_HARDWARE_TRIGGERED_STROBE 0xA
        #define PIT_COMMAND_BINARY_MODE 0x0
        #define PIT_COMMAND_BCD_MODE 0x1
        #define PIT_COMMAND_READ_BACK 0xC0
        #define PIT_COMMAND_READ_BACK_LATCH_COUNT 0x0
        #define PIT_COMMAND_READ_BACK_DONT_LATCH_COUNT 0x20
        #define PIT_COMMAND_READ_BACK_LATCH_STATUS 0x0
        #define PIT_COMMAND_READ_BACK_DONT_LATCH_STATUS 0x10
        #define PIT_COMMAND_READ_BACK_CHANNEL_0 0x2
        #define PIT_COMMAND_READ_BACK_CHANNEL_1 0x4
        #define PIT_COMMAND_READ_BACK_CHANNEL_2 0x8

        #define PIT_READ_BACK_STATUS_PIN(status_byte) status_byte >> 0x7
        #define PIT_READ_BACK_STATUS_NULL_COUNT(status_byte) status_byte >> 0x6
        #define PIT_READ_BACK_STATUS_ACCESS_MODE(status_byte) (status_byte >> 0x4) & 0x3
        #define PIT_READ_BACK_STATUS_OPERATING_MODE(status_byte) (status_byte >> 0x1) & 0x7

        #define PIT_STATUS_ACCESS_MODE_LATCH_COUNT 0x0
        #define PIT_STATUS_ACCESS_MODE_LOW_BYTE_ONLY 0x20
        #define PIT_SATTUS_ACCESS_MODE_HIGH_BYTE_ONLY 0x10
        #define PIT_STATUS_ACCESS_MODE_LOW_BYTE_HIGH_BYTE 0x30
        #define PIT_STATUS_OPERATING_MODE_INTERRUPT_ON_TERMINAL_COUNT 0x0
        #define PIT_STATUS_OPERATING_MODE_ONE_SHOT 0x8
        #define PIT_STATUS_OPERATING_MODE_RATE_GENERATOR 0x4
        #define PIT_STATUS_OPERATING_MODE_SQUARE_WAVE_GENERATOR 0xC
        #define PIT_STATUS_OPERATING_MODE_SOFTWARE_TRIGGERED_STROBE 0x2
        #define PIT_STATUS_OPERATING_MODE_HARDWARE_TRIGGERED_STROBE 0xA
        #define PIT_STATUS_BINARY_MODE 0x0
        #define PIT_STATUS_BCD_MODE 0x1

        void pit_interrupt_on_terminal_count(uint16_t);
        void pit_interrupt_rate_generator(uint16_t);
        uint16_t pit_get_counter_value(uint8_t);

#endif /** PIT_H */