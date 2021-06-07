#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "include/string/string.h"
#include "include/kernel/console.h"
#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;
static size_t terminal_tab_size;;
static bool wrapped = 0;

static void terminal_clear_line(size_t row) {
	for (size_t i = 0; i < VGA_WIDTH; i++) {
	terminal_put_entry_at(0, terminal_color, i, row);
	}
}

static void terminal_clear_line_from_column(size_t row, size_t column) {
	for (size_t i = column; i < VGA_WIDTH; i++) {
		terminal_put_entry_at(0, terminal_color, i, row);
	}
}

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_tab_size = 4;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_set_tab_size(size_t tab_size) {
	terminal_tab_size = tab_size;
}

void terminal_set_color(uint8_t color) {
	terminal_color = color;
}

void terminal_put_entry_at(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_put_char(char c) {
	unsigned char uc = c;
	if (uc == '\n') {
		if (++terminal_row == VGA_HEIGHT) {
			wrapped = 1;
			terminal_row = 0;
			terminal_clear_line(terminal_row);
		}
		terminal_column = 0;
		return;
	}
    else if (uc == '\t') {
		for (size_t i = 0; i < terminal_tab_size; i++) {
			terminal_put_entry_at(' ', terminal_color, terminal_column, terminal_row);
			if (++terminal_column > VGA_WIDTH) {
				terminal_column = 0;
				if (++terminal_row > VGA_HEIGHT) {
					wrapped = 1;
					terminal_row = 0;
				}
				else {
					terminal_row++;
				}
			}
		}
	}
    else {
		if(wrapped){
			terminal_clear_line_from_column(terminal_row, terminal_column + 1);
		}
		terminal_put_entry_at(uc, terminal_color, terminal_column, terminal_row);
	}
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			wrapped = 1;
			terminal_row = 0;
			terminal_clear_line(terminal_row);
		}
	}
}

void terminal_write(const char *data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		terminal_put_char(data[i]);
	}
}

void terminal_write_string(const char *data) {
	terminal_write(data, strlen(data));
}

void terminal_clear(){
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
