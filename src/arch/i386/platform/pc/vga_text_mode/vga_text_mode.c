#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/mmu.h>
#include <kernel/bootconsole.h>
#include <lib/string.h>
#include "vga_text_mode.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static const size_t VGA_TAB_SIZE = 4;
static uint16_t* const VGA_MEMORY = (uint16_t*) PHYSICAL_TO_VIRTUAL(0xB8000);
static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;
static uint8_t vga_fg_color;
static uint8_t vga_bg_color;
static uint16_t* vga_buffer;
static bool wrapped = 0;

int vga_init(void) {
	vga_row = 0;
	vga_column = 0;
	vga_fg_color = VGA_COLOR_LIGHT_GREY;
	vga_bg_color = VGA_COLOR_BLACK;
	vga_color = vga_entry_color(vga_fg_color, vga_bg_color);
	vga_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			vga_buffer[index] = vga_entry(' ', vga_color);
		}
	}
	return 0;
}

static void vga_put_entry_at(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	vga_buffer[index] = vga_entry(c, color);
}

static void vga_clear_line(size_t row) {
	for (size_t i = 0; i < VGA_WIDTH; i++) {
	vga_put_entry_at(0, vga_color, i, row);
	}
}

static void vga_clear_line_from_column(size_t row, size_t column) {
	for (size_t i = column; i < VGA_WIDTH; i++) {
		vga_put_entry_at(0, vga_color, i, row);
	}
}

void vga_put_char(char c) {
	unsigned char uc = c;
	if (uc == '\n') {
		if (++vga_row == VGA_HEIGHT) {
			wrapped = 1;
			vga_row = 0;
			vga_clear_line(vga_row);
		}
		vga_column = 0;
		return;
	}
    else if (uc == '\t') {
		for (size_t i = 0; i < VGA_TAB_SIZE; i++) {
			vga_put_entry_at(' ', vga_color, vga_column, vga_row);
			if (++vga_column > VGA_WIDTH) {
				vga_column = 0;
				if (++vga_row > VGA_HEIGHT) {
					wrapped = 1;
					vga_row = 0;
				}
				else {
					vga_row++;
				}
			}
		}
	}
    else {
		if(wrapped){
			vga_clear_line_from_column(vga_row, vga_column + 1);
		}
		vga_put_entry_at(uc, vga_color, vga_column, vga_row);
	}
	if (++vga_column == VGA_WIDTH) {
		vga_column = 0;
		if (++vga_row == VGA_HEIGHT) {
			wrapped = 1;
			vga_row = 0;
			vga_clear_line(vga_row);
		}
	}
}

void vga_put_string(const char* s, size_t size) {
	for (size_t i = 0; i < size; i++) {
		vga_put_char(s[i]);
	}
}

void vga_set_color(uint8_t fg_color, uint8_t bg_color) {
	vga_color = vga_entry_color(fg_color, bg_color);
}

void vga_set_fg_color(uint8_t fg_color) {
	vga_color = vga_entry_color(fg_color, vga_bg_color);
}

void vga_set_bg_color(uint8_t bg_color) {
	vga_color = vga_entry_color(vga_fg_color, bg_color);
}

void vga_clear() {
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			vga_buffer[index] = vga_entry(' ', vga_color);
		}
	}
}

bootconsole_t bootconsole_vga_text_mode = {
	.console_init = vga_init,
	.console_put_char = vga_put_char,
	.console_put_string = vga_put_string,
};