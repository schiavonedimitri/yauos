#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>
#include <stdint.h>

void console_initialize(void);
void console_clear(void);
void console_put_entry_at(unsigned char, uint8_t, size_t, size_t);
void console_put_char(char);
void console_write(const char*, size_t);
void console_write_string(const char*);
void console_set_tab_size(size_t);

#endif /** CONSOLE_H */
