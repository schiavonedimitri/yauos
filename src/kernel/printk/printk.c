#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/arch.h>
#include <kernel/bootconsole.h>
#include <kernel/printk.h>
#include <lib/string.h>

static void print_int_u32(uint32_t value, uint8_t base) {
	if (value == 0) {
		if (bootconsole_is_enabled()) {
			bootconsole_put_char((value % base) + '0');
		}
		return;
	}
	size_t size = 0;
	size_t tmp_size;
	uint32_t tmp = value;
	while (tmp) {
		size++;
		tmp /= base;
	}
	tmp_size = size;
	char buf[size + 1];
	buf[size] = 0;
	while (value) {
		buf[--tmp_size] = (value % base) + '0';
		value /= base;
	}
	if (bootconsole_is_enabled()) {
		bootconsole_put_string(&buf[0], strlen(&buf[0]));
	}
}

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
	size_t tmp_size;
	int32_t tmp = value;
	while (tmp) {
		size++;
		tmp /= base;
	}
	tmp_size = size;
	char buf[size + 1];
	buf[size] = 0;
	while (value) {
		buf[--tmp_size] = (value % base) + '0';
		value /= base;
	}
	if (bootconsole_is_enabled()) {
		bootconsole_put_string(&buf[0], strlen(&buf[0]));
	}
}

static void print_int_u64(uint64_t value, uint8_t base) {
	if (value == 0) {
		if (bootconsole_is_enabled()) {
			bootconsole_put_char((value % base) + '0');
		}
		return;
	}
	size_t size = 0;
	size_t tmp_size;
	uint64_t tmp = value;
	while (tmp) {
		size++;
		tmp /= base;
	}
	tmp_size = size;
	char buf[size + 1];
	buf[size] = 0;
    while (value) {
		buf[--tmp_size] = (value % base) + '0';
		value /= base;
	}
	if (bootconsole_is_enabled()) {
		bootconsole_put_string(&buf[0], strlen(&buf[0]));
	}
}

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
	size_t tmp_size;
	int64_t tmp = value;
	while (tmp) {
		size++;
		tmp /= base;
	}
	tmp_size = size;
	char buf[size + 1];
	buf[size] = 0;
    while (value) {
		buf[--tmp_size] = (value % base) + '0';
		value /= base;
	}
	if (bootconsole_is_enabled()) {
		bootconsole_put_string(&buf[0], strlen(&buf[0]));
	}
}

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

void _printk(bool panic, const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);
	while (*format) {
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
