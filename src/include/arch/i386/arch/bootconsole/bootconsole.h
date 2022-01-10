#ifndef BOOTCONSOLE_H
#define BOOTCONSOLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum bootconsole_type {
	BOOTCONSOLE_MEM = 0,
	BOOTCONSOLE_VGA = 1,
	BOOTCONSOLE_SERIAL = 2,
} bootconsole_type_t;

typedef struct bootconsole {
	void (*console_init) (void);
	void (*console_put_char) (char);
	void (*console_put_string) (const char*, size_t);
} bootconsole_t;

void bootconsole_disable();
bool bootconsole_is_enabled();
void bootconsole_init(bootconsole_type_t);
void bootconsole_put_char(char);
void bootconsole_put_string(const char*, size_t);

#endif /** BOOTCONSOLE_H */
