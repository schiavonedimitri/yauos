#include <stddef.h>
#include <stdint.h>
#include <arch/arch.h>
#include <arch/bootconsole/bootconsole.h>
#include <arch/cpu/io.h>

#define COM1_PORT 0x3F8

inline static int is_transmit_empty() {
   return inb(COM1_PORT + 5) & 0x20;
}

void bootconsole_serial_init() {
	outb(COM1_PORT + 1, 0x00);
	outb(COM1_PORT + 3, 0x80);
	outb(COM1_PORT + 0, 0x01);
	outb(COM1_PORT + 1, 0x00);
	outb(COM1_PORT + 3, 0x03);
	outb(COM1_PORT + 2, 0xC7);
	outb(COM1_PORT + 4, 0x0B);
	outb(COM1_PORT + 4, 0x1E);
	outb(COM1_PORT + 0, 0xAA);
	if(inb(COM1_PORT + 0) != 0xAA) {
		arch_halt();
	}
	outb(COM1_PORT + 4, 0x0F);
}

void bootconsole_serial_put_char(char c) {
	while (is_transmit_empty() == 0);
	outb(COM1_PORT, c);
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
