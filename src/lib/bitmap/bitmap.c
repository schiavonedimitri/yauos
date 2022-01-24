#include <stddef.h>
#include <stdint.h>
#include "bitmap.h"
#include <kernel/printk.h>

void bitmap_set(void *bitmap, int bit) {
	unsigned long *b_map = (unsigned long*) bitmap;
	b_map[bit / BITMAP_BITS] |= (1 << (bit % BITMAP_BITS));
}

void bitmap_unset(void *bitmap, int bit) {
	unsigned long *b_map = (unsigned long*) bitmap;
	b_map[bit / BITMAP_BITS] &= ~(1 << (bit % BITMAP_BITS));
}

int bitmap_test(void *bitmap, int bit) {
	unsigned long *b_map = (unsigned long*) bitmap;
	return (b_map[bit / BITMAP_BITS] & (1 << (bit % BITMAP_BITS))) >> (bit % BITMAP_BITS);
}

int bitmap_first_unset(void *bitmap, size_t size) {
	unsigned long *b_map = (unsigned long*) bitmap;
	size_t rem_bits = size % BITMAP_BITS;
	for (size_t i = 0; i < size / BITMAP_BITS; i++) {
		if (b_map[i] != BITMAP_FULL_MASK) {
			for (size_t j = 0; j < BITMAP_BITS; j++) {
				if (!(b_map[i] & (1 << j))) {
                    return (i * BITMAP_BITS) + j;
				}
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
