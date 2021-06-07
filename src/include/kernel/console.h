#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>
#include <stdint.h>

void terminal_initialize(void);
void terminal_clear(void);
void terminal_put_entry_at(unsigned char, uint8_t, size_t, size_t);
void terminal_put_char(char);
void terminal_write(const char*, size_t);
void terminal_write_string(const char*);
void terminal_set_tab_size(size_t);

#endif /** CONSOLE_H */
