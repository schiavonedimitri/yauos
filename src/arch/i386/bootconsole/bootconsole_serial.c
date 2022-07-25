#include <stddef.h>
#include <stdint.h>
#include <arch/arch.h>
#include <arch/cpu/io.h>
#include <kernel/bootconsole.h>
#include "bootconsole_serial.h"

inline static int transmit_empty() {
   return inb(LINE_STATUS_REGISTER(COM1_PORT)) & 0x20;
}

int bootconsole_serial_init() {
	
	// Disable interrupts.
	
	outb(INTERRUPT_ENABLE_REGISTER(COM1_PORT), INTERRUPT_DISABLE);
	
	// Enable DLAB so that the first and second ports become the low and high byte of the baud rate divisor.
	
	outb(LINE_CONTROL_REGISTER(COM1_PORT), DLAB_ENABLE);
	
	// Set low byte of the baud rate divisor to 1
	
	outb(DATA_REGISTER(COM1_PORT), 0x1);
	
	// Set the high byte of the baud rate divisor to 0 so that the baud rate is set to the maximum.
	
	outb(INTERRUPT_ENABLE_REGISTER(COM1_PORT), 0x0);
	
	// Set serial mode to character length of 8 with parity none and 1 stop bits.
	
	outb(LINE_CONTROL_REGISTER(COM1_PORT), CHAR_LENGTH_8 | PARITY_NONE | STOP_BITS_1);
	
	// Enable FIFO, clear both send and receive FIFO with a 14 byte threshold.
	
	outb(INTERRUPT_IDENTIFICATION_AND_FIFO_REGISTER(COM1_PORT), ENABLE_FIFOS | CLEAR_RECEIVE_FIFO | CLEAR_TRANSMIT_FIFO | FIFO_14_BYTE_THRESHOLD);
	
	// Set data terminal ready, request to send and out 2.
	
	outb(MODEM_CONTROL_REGISTER(COM1_PORT), DATA_TERMINAL_READY | REQUEST_TO_SEND | OUT_2);
	
	// Set in loopback mode to test serial is functional.
	
	outb(MODEM_CONTROL_REGISTER(COM1_PORT), REQUEST_TO_SEND | OUT_1 | OUT_2 | LOOP);
	
	// Send test data.
	
	outb(DATA_REGISTER(COM1_PORT), 0xAA);
	
	// Check data received is the same as data sent.
	
	if (inb(DATA_REGISTER(COM1_PORT)) != 0xAA) {
		return -1;
	}
	
	// Set serial device operational.
	
	outb(MODEM_CONTROL_REGISTER(COM1_PORT), DATA_TERMINAL_READY | REQUEST_TO_SEND | OUT_1 | OUT_2);
	return 0;
}

void bootconsole_serial_put_char(char c) {
	while (transmit_empty() == 0);
	outb(DATA_REGISTER(COM1_PORT), c);
}

void bootconsole_serial_put_string(const char* s, size_t size) {
	for (size_t i = 0; i < size; i++) {
		bootconsole_serial_put_char(s[i]);
	}
}

bootconsole_t bootconsole_serial = {
	.console_init = bootconsole_serial_init,
	.console_put_char = bootconsole_serial_put_char,
	.console_put_string = bootconsole_serial_put_string,
};