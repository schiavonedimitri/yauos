#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <lib/string/string.h>
#include <arch/vga.h>
#include <kernel/printk.h>

void halt(void);

static void print_int_u32(uint32_t value, uint8_t base) {
	if (value == 0) {
		vga_put_c((value % base) + '0');
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
	char buf[size];
	buf[size] = 0;
    while (value) {
        buf[--tmp_size] = (value % base) + '0';
        value /= base;
    }
    vga_write(&buf[0], strlen(&buf[0]));
}

static void print_int_32(int32_t value, uint8_t base) {
	if (value < 0) {
		vga_put_c('-');
	}
	value = (value < 0 ? -value : value);
	if (value == 0) {
		vga_put_c((value % base) + '0');
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
	char buf[size];
	buf[size] = 0;
    while (value) {
        buf[--tmp_size] = (value % base) + '0';
        value /= base;
    }
    vga_write(&buf[0], strlen(&buf[0]));
}

static void print_int_u64(uint64_t value, uint8_t base) {
	if (value == 0) {
		vga_put_c((value % base) + '0');
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
	char buf[size];
	buf[size] = 0;
    while (value) {
        buf[--tmp_size] = (value % base) + '0';
        value /= base;
    }
    vga_write(&buf[0], strlen(&buf[0]));
}

static void print_int_64(int64_t value, uint8_t base) {
	if (value < 0) {
		vga_put_c('-');
	}
	value = (value < 0 ? -value : value);
	if (value == 0) {
		vga_put_c((value % base) + '0');
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
	char buf[size];
	buf[size] = 0;
    while (value) {
        buf[--tmp_size] = (value % base) + '0';
        value /= base;
    }
    vga_write(&buf[0], strlen(&buf[0]));
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
	vga_put_c('0');
	vga_put_c('x');
	bool first_non_zero = 0;
	for (size_t i = 0, j = sizeof(uint32_t) * 8 - 4; i < sizeof(uint32_t) * 2; i++, j -= 4) {
		char c = index[((value & (((uint32_t)0xF) << j)) >> j)];
		if (c != '0') {
			first_non_zero = 1;
		}
		if (first_non_zero) {
			vga_put_c(c);
		}
	}
	if (!first_non_zero) {
		vga_put_c('0');
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
	vga_put_c('0');
	vga_put_c('x');
	bool first_non_zero = 0;
	for (size_t i = 0, j = sizeof(uint64_t) * 8 - 4; i < sizeof(uint64_t) * 2; i++, j -= 4) {
        char c = index[((value & (((uint64_t) 0xF) << j)) >> j)];
        if (c != '0') {
            first_non_zero = 1;
        }
        if (first_non_zero) {
            vga_put_c(c);
        }
    }
    if (!first_non_zero) {
        vga_put_c('0');
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
			vga_write(format, amount);
			format += amount;
			continue;
		}
		const char *format_begun_at = format++;
		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int);
			vga_put_c(c);
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
			vga_write(str, strlen(str));
		}
		else {
			format = format_begun_at;
            size_t len = strlen(format);
			vga_write(format, len);
			format += len;
		}
	}
	va_end(parameters);
	if (panic) {
		halt();
	}
}
