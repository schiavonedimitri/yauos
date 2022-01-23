#ifndef BOOTINFO_H
#define BOOTINFO_H

#include <stddef.h>
#include <stdint.h>
#include <arch/arch.h>

/* This header contains the definition of the structure of boot parameters the kernel expects to be passed. This is necessary for the kernel to be architecture independent.
 * All architectures that want to support the kernel need to pass boot information in a way the kernel understands it.
 */

#define MEMORY_RESERVED 0
#define MEMORY_AVAILABLE 1
#define MEMORY_UNSPEC 2

typedef struct karg {
	char* key;
	char* value;
} karg_t;

typedef struct memory_entry {
	uint64_t base_addr;
	size_t length;
	uint8_t type;
} memory_entry_t;

typedef struct bootinfo {
	size_t karg_entries;
	karg_t *karg_entry;
	size_t memory_map_entries;
	memory_entry_t *memory_map_entry;
	size_t memory_size;
} bootinfo_t;

#endif /** BOOTINFO_H */
