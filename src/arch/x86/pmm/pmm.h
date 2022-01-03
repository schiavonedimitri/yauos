#ifndef _PMM_H
#define _PMM_H

#include <stdint.h>
#include <arch/mmu.h>
#include <arch/types.h>

#define BLOCK_SIZE PAGE_SIZE

extern virt_addr_t _KERNEL_START_;
extern virt_addr_t _KERNEL_END_;

char *memory_map_lookup_table[] = {
	"",
	"Available",
	"Reserved",
	"Acpi reclaimable",
	"Nvs",
	"Bad ram"
};

typedef struct memory_entry {
	uint64_t start_addr;
	uint64_t end_addr;
	uint64_t length;
} memory_entry_t;

#endif /** _PMM_H */
