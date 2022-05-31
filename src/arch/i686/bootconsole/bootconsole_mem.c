#include <stddef.h>
#include <stdint.h>
#include <kernel/bootconsole.h>
#include <lib/string.h>

#define BUF_SIZE 4096

static char buffer[BUF_SIZE];
static size_t cur_index;
static bool already_initialized = 0;

int bootconsole_mem_init() {
	if (!already_initialized) {
		memset(buffer, 0x0, BUF_SIZE);
		cur_index = 0;
		already_initialized = 1;
	}
	return 1;
}

void bootconsole_mem_put_char(char c) {
	buffer[cur_index % BUF_SIZE] = c;
	cur_index++;
	cur_index %= BUF_SIZE;
}

void bootconsole_mem_put_string(const char* s, size_t size) {
	for (size_t i = 0; i < size; i++) {
		bootconsole_mem_put_char(s[i]);
	}
}

void bootconsole_mem_flush_buffer(char* buf) {
	memcpy(buf, buffer, cur_index);
}

size_t bootconsole_mem_get_number_buffered_items() {
	return cur_index;
}

bootconsole_t bootconsole_mem = {
	.console_init = bootconsole_mem_init,
	.console_put_char = bootconsole_mem_put_char,
	.console_put_string = bootconsole_mem_put_string,
};
