#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/arch.h>
#include <kernel/bootconsole.h>
#include <kernel/printk.h>
#include <lib/string.h>

/*
 * This routine prints an unsigned integer of 32 bits by converting it into its ASCII representation.
 * The basic formula to get the ascii value from the integer value is: value % base + '0'.
 */
static void print_int_u32(uint32_t value, uint8_t base) {
	// If the number si just one digit print it and return.
	if (value == 0) {
		if (bootconsole_is_enabled()) {
			bootconsole_put_char((value % base) + '0');
		}
		return;
	}
	size_t size = 0;
	uint32_t tmp = value;
	// Calculate the size for the buffer to hold the ASCII string of the number.
	while (tmp) {
		size++;
		tmp /= base;
	}
	// Add one for the NULL character.
	char buf[size + 1];
	buf[size] = 0;
	// Fill the created buffer in reverse order.
	while (value) {
		buf[--size] = (value % base) + '0';
		value /= base;
	}
	// Finally print the ASCII string containing the number.
	if (bootconsole_is_enabled()) {
		bootconsole_put_string(&buf[0], strlen(&buf[0]));
	}
}

// This routine is the same as the unsigned one except that it checks if the number has a sign and prepends it prior to printing.

static void print_int_32(int32_t value, uint8_t base) {
	if (value < 0) {
		if (bootconsole_is_enabled()) {
			bootconsole_put_char('-');
		}
	}
	value = (value < 0 ? -value : value);
	if (value == 0) {
		if (bootconsole_is_enabled()) {
			bootconsole_put_char((value % base) + '0');
		}
		return;
	}
	size_t size = 0;
	int32_t tmp = value;
	while (tmp) {
		size++;
		tmp /= base;
	}
	char buf[size + 1];
	buf[size] = 0;
	while (value) {
		buf[--size] = (value % base) + '0';
		value /= base;
	}
	if (bootconsole_is_enabled()) {
		bootconsole_put_string(&buf[0], strlen(&buf[0]));
	}
}

// This routine is equivalent to print_int_u32 but for 64 bits numbers.

static void print_int_u64(uint64_t value, uint8_t base) {
	if (value == 0) {
		if (bootconsole_is_enabled()) {
			bootconsole_put_char((value % base) + '0');
		}
		return;
	}
	size_t size = 0;
	uint64_t tmp = value;
	while (tmp) {
		size++;
		tmp /= base;
	}
	char buf[size + 1];
	buf[size] = 0;
    while (value) {
		buf[--size] = (value % base) + '0';
		value /= base;
	}
	if (bootconsole_is_enabled()) {
		bootconsole_put_string(&buf[0], strlen(&buf[0]));
	}
}

// This routine is equivalent to print_int_32 but for 64 bit numbers.

static void print_int_64(int64_t value, uint8_t base) {
	if (value < 0) {
		if (bootconsole_is_enabled()) {
			bootconsole_put_char('-');
		}
	}
	value = (value < 0 ? -value : value);
	if (value == 0) {
		if (bootconsole_is_enabled()) {
			bootconsole_put_char((value % base) + '0');
		}
		return;
	}
	size_t size = 0;
	int64_t tmp = value;
	while (tmp) {
		size++;
		tmp /= base;
	}
	char buf[size + 1];
	buf[size] = 0;
    while (value) {
		buf[--size] = (value % base) + '0';
		value /= base;
	}
	if (bootconsole_is_enabled()) {
		bootconsole_put_string(&buf[0], strlen(&buf[0]));
	}
}

/*
 * This routine prints a 32 bit integer as hexadecimal number.
 * It works by checking what value of the 32 bit word has each single nibble (4 bits for the not so savy).
 * Then that value is used as an index into the index array which contains the ASCII value for 0 to F.
 */

static void print_hex_32(uint32_t value) {
    char index[16] = {
		'0',
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'A',
		'B',
		'C',
		'D',
		'E',
		'F'
	};
	if (bootconsole_is_enabled()) {
		bootconsole_put_char('0');
		bootconsole_put_char('x');
	}
	bool first_non_zero = 0;
	for (size_t i = 0, j = sizeof(uint32_t) * 8 - 4; i < sizeof(uint32_t) * 2; i++, j -= 4) {
		char c = index[((value & (((uint32_t) 0xF) << j)) >> j)];
		// This is done for removing any leading zeroes.
		if (c != '0') {
			first_non_zero = 1;
		}
		if (first_non_zero) {
			if (bootconsole_is_enabled()) {
				bootconsole_put_char(c);
			}
		}
	}
	if (!first_non_zero) {
		if (bootconsole_is_enabled()) {
				bootconsole_put_char('0');
			}
	}
}

// This routine is equivalent to print_hex_32 but is for 64 bits.

static void print_hex_64(uint64_t value) {
    char index[16] = {
		'0',
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'A',
		'B',
		'C',
		'D',
		'E',
		'F'
	};
	if (bootconsole_is_enabled()) {
		bootconsole_put_char('0');
		bootconsole_put_char('x');
	}
	bool first_non_zero = 0;
	for (size_t i = 0, j = sizeof(uint64_t) * 8 - 4; i < sizeof(uint64_t) * 2; i++, j -= 4) {
		char c = index[((value & (((uint64_t) 0xF) << j)) >> j)];
		// This is done for removing any leading zeroes.
		if (c != '0') {
			first_non_zero = 1;
		}
		if (first_non_zero) {
			if (bootconsole_is_enabled()) {
				bootconsole_put_char(c);
			}
		}
	}
	if (!first_non_zero) {
		if (bootconsole_is_enabled()) {
				bootconsole_put_char('0');
			}
	}
}

/*
 * This function is not used directly. See printk.h for the definition of appropriate macros to use instead.
 * This simply scans the format string for known and implemented patterns and calls the appropriate functions
 * to print the corresponding values. If the format string is just a normal string, it is interpreted as such and 
 * printed.
 */

void _printk(bool panic, const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);
	// While format is not a NULL character
	while (*format) {
		// This first case handles printing the format string from the first character up to % excluded as a normal string.
		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%') {
				format++;
			}
			size_t amount = 1;
			while (format[amount] && format[amount] != '%') {
				amount++;
			}
			if (bootconsole_is_enabled()) {
				bootconsole_put_string(format, amount);
			}
			format += amount;
			continue;
		}
		// Save the position after the % character in case it isn't one of those below (meeaning it's not implemented).
		const char *format_begun_at = format++;
		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int);
			if (bootconsole_is_enabled()) {
				bootconsole_put_char(c);
			}
		}
		else if (*format == 'd') {
			format++;
			uint32_t d = (uint32_t) va_arg(parameters, uint32_t);
			print_int_u32(d, 10);
		}
		else if (*format == 'l' && *(format + 1) == 'd') {
			format += 2;
			uint64_t d = (uint64_t) va_arg(parameters, uint64_t);
			print_int_u64(d, 10);
		}
		else if (*format == 's' && *(format + 1) == 'd') {
			format += 2;
			int32_t d = (int32_t) va_arg(parameters, int32_t);
			print_int_32(d, 10);
		}
		else if (*format == 's' && *(format + 1) == 'l' && *(format + 2) == 'd') {
			format += 3;
			int64_t d = (int64_t) va_arg(parameters, int64_t);
			print_int_64(d, 10);
		}
		else if (*format == 'x') {
			format++;
			uint32_t d = (uint32_t) va_arg(parameters, uint32_t);
			print_hex_32(d);
		}
		else if (*format == 'l' && *(format + 1) == 'x') {
			format += 2;
			uint64_t d = (uint64_t) va_arg(parameters, uint64_t);
			print_hex_64(d);
		}
		else if (*format == 'b') {
			format++;
			uint32_t d = (uint32_t) va_arg(parameters, uint32_t);
			print_int_u32(d, 2);
		}
		else if (*format == 'l' && *(format + 1) == 'b') {
			format += 2;
			uint64_t d = (uint64_t) va_arg(parameters, uint64_t);
			print_int_u64(d, 2);
		}
		else if (*format == 's') {
			format++;
			const char *str = va_arg(parameters, const char*);
			if (bootconsole_is_enabled()) {
				bootconsole_put_string(str, strlen(str));
			}
		}
		// Print the string or character after the '%' character because it wasn't found above.
		else {
			format = format_begun_at;
            size_t len = strlen(format);
			if (bootconsole_is_enabled()) {
				bootconsole_put_string(format, len);
			}
			format += len;
		}
	}
	va_end(parameters);
	// This implements the panic() macro like function to print additional information about the error and halt the machine.
	if(panic) {
		if (bootconsole_is_enabled()) {
			bootconsole_put_string(" File: ", strlen(" File: "));
			bootconsole_put_string(__FILENAME__, strlen(__FILENAME__));
			bootconsole_put_string(" Line: ", strlen(" Line: "));
			print_int_u32(__LINE__, 10);
			bootconsole_put_string(" Function: ", strlen(" Function: "));
			bootconsole_put_string(__func__, strlen(__func__));
			bootconsole_put_char('\n');
		}
		arch_halt();
	}
}
