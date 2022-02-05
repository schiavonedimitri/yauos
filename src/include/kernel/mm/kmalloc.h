#ifndef KMALLOC_H
#define KMALLOC_H

#include <arch/types.h>

#ifndef KERNEL_HEAP_SIZE
#define KERNEL_HEAP_SIZE 0x4000000
#endif

extern uintptr_t _KERNEL_END_;

typedef long Align;

typedef union header {
	struct {
	union header *ptr;
	size_t size;
	} s;
	Align x;
} Header;

// This is set so that the minimum allocation request to the physical memory manger will be the size of a page which is the natural allocation size of the system.
#define MORECORE_DEFAULT PAGE_SIZE / sizeof(Header)

ssize_t k_malloc_init();
void *k_malloc(size_t);
void *k_zmalloc(size_t);
void k_free(void*);

#endif /** KMALLOC_H */