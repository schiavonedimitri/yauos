#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

void vga_initialize(void);
void vga_clear(void);
void vga_put_c(char);
void vga_write(const char*, size_t);

#endif /** VGA_H */
