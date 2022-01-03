#include <stddef.h>
#include <stdint.h>
#include <arch/types.h>
#include "bitmap.h"

void bitmap_set(void *bitmap, int bit) {
	uintptr_t *b_map = (uintptr_t*) bitmap;
	b_map[bit / BITMAP_BITS] |= (1 << (bit % BITMAP_BITS));
}

void bitmap_unset(void *bitmap, int bit) {
	virt_addr_t *b_map = (uint32_t*) bitmap;
	b_map[bit / BITMAP_BITS] &= ~(1 << (bit % BITMAP_BITS));
}

int bitmap_test(void *bitmap, int bit) {
	virt_addr_t *b_map = (virt_addr_t*) bitmap;
	return b_map[bit / BITMAP_BITS] & (1 << (bit % BITMAP_BITS));
}

int bitmap_first_unset(void *bitmap, size_t size) {
	virt_addr_t *b_map = (virt_addr_t*) bitmap;
	size_t rem_bits = size % BITMAP_BITS;
	for (size_t i = 0; i < size / BITMAP_BITS; i++) {
		if (b_map[i] != BITMAP_FULL_MASK)
			for (size_t j = 0; j < BITMAP_BITS; j++) {
				if (!(b_map[i] & (1 << j))) {
                    return (i * BITMAP_BITS) + j;
				}
			}
	}
	if (rem_bits) {
		for (size_t j = 0; j < rem_bits; j++) {
			if (!(b_map[size / BITMAP_BITS] & (1 << j))) {
				return ((size / BITMAP_BITS) * BITMAP_BITS) + j;
			}
		}
    }
	return -1;
}
