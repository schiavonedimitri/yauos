#include <stddef.h>
#include <stdint.h>
#include <arch/types.h>

void bitmap_set(void *bitmap, int bit) {
	virt_addr_t *b_map = (virt_addr_t*) bitmap;
	b_map[bit / 32] |= (1 << (bit % 32));
}

void bitmap_unset(void *bitmap, int bit) {
	virt_addr_t *b_map = (uint32_t*) bitmap;
	b_map[bit / 32] &= ~(1 << (bit % 32));
}

int bitmap_test(void *bitmap, int bit) {
	virt_addr_t *b_map = (virt_addr_t*) bitmap;
	return b_map[bit / 32] & (1 << (bit % 32));
}

int bitmap_first_unset(void *bitmap, size_t size) {
	virt_addr_t *b_map = (virt_addr_t*) bitmap;
	size_t rem_bits = size % 32;
	for (size_t i = 0; i < size / 32; i++) {
		if (b_map[i] != 0XFFFFFFFF)
			for (size_t j = 0; j < 32; j++) {
				if (!(b_map[i] & (1 << j))) {
                    return (i * 32) + j;
				}
			}
	}
	if (rem_bits) {
		for (size_t j = 0; j < rem_bits; j++) {
			if (!(b_map[size / 32] & (1 << j))) {
				return ((size / 32) * 32) + j;
			}
		}
    }
	return -1;
}
