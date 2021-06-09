#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "include/string/string.h"
#include "include/kernel/console.h"
#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xC03FF000;
static size_t console_row;
static size_t console_column;
static uint8_t console_color;
static uint16_t* console_buffer;
static size_t console_tab_size;;
static bool wrapped = 0;

static void console_clear_line(size_t row) {
	for (size_t i = 0; i < VGA_WIDTH; i++) {
	console_put_entry_at(0, console_color, i, row);
	}
}

static void console_clear_line_from_column(size_t row, size_t column) {
	for (size_t i = column; i < VGA_WIDTH; i++) {
		console_put_entry_at(0, console_color, i, row);
	}
}

void console_initialize(void) {
	console_row = 0;
	console_column = 0;
	console_tab_size = 4;
	console_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	console_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			console_buffer[index] = vga_entry(' ', console_color);
		}
	}
}

void console_set_tab_size(size_t tab_size) {
	console_tab_size = tab_size;
}

void console_set_color(uint8_t color) {
	console_color = color;
}

void console_put_entry_at(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	console_buffer[index] = vga_entry(c, color);
}

void console_put_char(char c) {
	unsigned char uc = c;
	if (uc == '\n') {
		if (++console_row == VGA_HEIGHT) {
			wrapped = 1;
			console_row = 0;
			console_clear_line(console_row);
		}
		console_column = 0;
		return;
	}
    else if (uc == '\t') {
		for (size_t i = 0; i < console_tab_size; i++) {
			console_put_entry_at(' ', console_color, console_column, console_row);
			if (++console_column > VGA_WIDTH) {
				console_column = 0;
				if (++console_row > VGA_HEIGHT) {
					wrapped = 1;
					console_row = 0;
				}
				else {
					console_row++;
				}
			}
		}
	}
    else {
		if(wrapped){
			console_clear_line_from_column(console_row, console_column + 1);
		}
		console_put_entry_at(uc, console_color, console_column, console_row);
	}
	if (++console_column == VGA_WIDTH) {
		console_column = 0;
		if (++console_row == VGA_HEIGHT) {
			wrapped = 1;
			console_row = 0;
			console_clear_line(console_row);
		}
	}
}

void console_write(const char *data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		console_put_char(data[i]);
	}
}

void console_write_string(const char *data) {
	console_write(data, strlen(data));
}

void console_clear(){
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			console_buffer[index] = vga_entry(' ', console_color);
		}
	}
}
