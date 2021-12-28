#ifndef PMM_H
#define PMM_H
#include <stddef.h>
#include <stdint.h>
#include <arch/multiboot2.h>

typedef struct memory_entry {
	uint64_t start_addr;
	uint64_t end_addr;
	uint64_t length;
} memory_entry_t;

void pmm_init(multiboot2_information_header_t *m_boot2_info);


#endif /** PMM_H */
