#ifndef _BOOTMEM_H
#define _BOOTMEM_H

#include <stdint.h>
#include <arch/types.h>

/*
 * Used by morecore() to request memory chunks. By default it requests 1Kb chunks per call.
 * Shouldn't be higher than EARLY_HEAP_SIZE / sizeof(Header) or allocations will fail due to the pool being limited to EARLY_HEAP_SIZE bytes.
 */

#define MORECORE_DEFAULT 128

extern virt_addr_t bootmem_start;
extern virt_addr_t bootmem_end;

typedef long Align;

typedef union header {
	struct {
	union header *ptr;
	size_t size;
	} s;
	Align x;
} Header;

#endif /** _BOOTMEM_H */
