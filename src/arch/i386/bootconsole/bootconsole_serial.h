#ifndef _BOOTCONSOLE_SERIAL_H
        #define _BOOTCONSOLE_SERIAL_H

        #define COM1_PORT 0x3F8
        #define COM2_PORT 0x2F8

        #define DATA_REGISTER(port) (port + 0x0)
        #define INTERRUPT_ENABLE_REGISTER(port) (port + 0x1)
        #define INTERRUPT_IDENTIFICATION_AND_FIFO_REGISTER(port) (port + 0x2)
        #define LINE_CONTROL_REGISTER(port) (port + 0x3)
        #define MODEM_CONTROL_REGISTER(port) (port + 0x4)
        #define LINE_STATUS_REGISTER(port) (port + 0x5)
        #define MODEM_STATUS_REGISTER(port) (port + 0x6)
        #define SCRATCH_REGISTER(port) (port + 0x7)

        #define DLAB_ENABLE 0x80
        #define DLAB_DISABLE 0x0
        #define CHAR_LENGTH_5 0x0
        #define CHAR_LENGTH_6 0x1
        #define CHAR_LENGTH_7 0x2
        #define CHAR_LENGTH_8 0x3
        #define STOP_BITS_1 0x0
        #define STOP_BITS_1_5_2 0x4
        #define PARITY_NONE 0x0
        #define PARITY_ODD 0x8
        #define PARITY_EVEN 0x18
        #define PARITY_MARK 0x28
        #define PARITY_SPACE 0x38
        #define INTERRUPT_ENABLE 0x1
        #define INTERRUPT_DISABLE 0x0
        #define INTERRUPT_ON_DATA_AVAILABLE 0x1
        #define INTERURPT_ON_TRANSMITTER_EMPTY 0x2
        #define INTERRUPT_ON_BREAK_ERROR 0x4
        #define INTERRUPT_ON_STATUS_CHANGE 0x8
        #define ENABLE_FIFOS 0x1
        #define CLEAR_RECEIVE_FIFO 0x2
        #define CLEAR_TRANSMIT_FIFO 0x4
        #define FIFO_14_BYTE_THRESHOLD 0xC0
        #define DATA_TERMINAL_READY 0x1
        #define REQUEST_TO_SEND 0x2
        #define OUT_1 0x4
        #define OUT_2 0x8
        #define LOOP 0x10
        #define DATA_READY 0x1
        #define DATA_OVERRUN 0x2
        #define PARITY_ERROR 0x4
        #define FRAMING_ERROR 0x8
        #define BREAK_INDICATOR 0x10
        #define TRANSMITTER_HOLDING_REGISTER_EMPTY 0x20
        #define TRANSMITTER_EMPTY 0x40
        #define IMPENDING_ERROR 0x80
        #define DELTA_CLEAR_TO_SEND 0x0
        #define DELTA_DATA_SET_READY 0x2
        #define TRAILING_EDGE_RING_INDICATOR 0x4
        #define DELTA_DATA_CARRIER_DETECT 0x8
        #define CLEAR_TO_SEND 0x10
        #define DATA_SET_READY 0x20
        #define RING_INDICATOR 0x40
        #define DATA_CARRIER_DETECT 0x80

#endif /** _BOOTCONSOLE_SERIAL_H */