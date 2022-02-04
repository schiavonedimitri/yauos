#ifndef KMALLOC_H
#define KMALLOC_H

#include <arch/types.h>

// 64Mb of heap size
#define MAX_HEAP_SIZE 0x4000000

extern uintptr_t _KERNEL_END_;

typedef long Align;

typedef union header {
	struct {
	union header *ptr;
	size_t size;
	} s;
	Align x;
} Header;

#define MORECORE_DEFAULT PAGE_SIZE

ssize_t kmalloc_init();
void* alloc_page();
void *kmalloc(size_t);
void kfree(void*);

#endif /** KMALLOC_H */