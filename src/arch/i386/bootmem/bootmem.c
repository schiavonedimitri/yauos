#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/types.h>
#include <lib/string.h>
#include "bootmem.h"

/*
 * This an implementation of the K&R malloc for use during early boot. Although a heap early in the boot is overkill it is indeed convenient.
 * Programming without dynamic memory is always more convoluted and the rationale for this is the following:
 * No assumptions can be made about memory (not even memory past the end of the kernel) during boot before inspecting
 * a memory map provided by firmware or by a predecessor in the boot chain.
 * Due to this, the allocator draws from a pool of EARLY_HEAP_SIZE reserved in the kernel .bss section which, instead, is guarenteed.
 * (Because this is an elf kernel, if not enough memory was available for the loader it wouldn't have loaded us).
 * This is mostly used during early stage in the initialization process, before the memory manager is up and running and for aiding at bootstrapping
 * the memory manager itself. A simple watermark allocator could've been used since this memory will never be freed.
 * Nevertheless the convenience of having a free and since the allocator manages such a small memory pool should add little to no overhead for great added functionality.
 */

// Heap_brk is initially already aligned to a page boundary in boot.S
static virt_addr_t heap_brk = (virt_addr_t) &bootmem_start;
static virt_addr_t heap_end = (virt_addr_t) &bootmem_end;
static Header base;
static Header *freep;

/*
 * Increases the heap break by the requested amount aligned to max_align_t.
 * Returns -1 in case of failure.
 */

static void* b_sbrk(size_t amount) {
	if ((size_t) heap_brk + amount > (size_t) heap_end) {
		return (void*) -1;
	}
	void *prev_brk = (void*) heap_brk;
	heap_brk = (virt_addr_t) ((size_t) heap_brk + amount);
	return prev_brk;
}

/*
 * Frees the memory pointed by ap.
 * Does not check for double free or anything else. It is assumed to be correctly used since it is used by the kernel itself and has no interactions
 * with user space.
 */

void b_free(void *ap) {
	Header *bp, *p;
	bp = (Header*) ap - 1;
	for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
		if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) {
			break;
		}
	}
	if (bp + bp->s.size == p->s.ptr) {
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	}
	else {
		bp->s.ptr = p->s.ptr;
	}
	if (p + p->s.size == bp) {
		p->s.size += bp->s.size;
		p->s.ptr = bp->s.ptr;
	}
	else {
		p->s.ptr = bp;
	}
	freep = p;
}

static Header* b_morecore(size_t n_units) {
	void *p;
	Header *hp;
	if (n_units < MORECORE_DEFAULT) {
		n_units = MORECORE_DEFAULT;
	}
	p = b_sbrk(n_units * sizeof(Header));
	if (p == (void*) -1) {
		return NULL;
	}
	hp = (Header*) p;
	hp->s.size = n_units;
	b_free((void*) (hp + 1));
	return freep;
}

/*
 * Returns the starting address of the requested memory.
 * In case of failure NULL is returned.
 */

void* b_malloc(size_t n_bytes) {
	Header *p, *prevp;
	size_t n_units;
	n_units = (n_bytes + sizeof(Header) - 1) / sizeof(Header) + 1;
	if ((prevp = freep) == 0) {
		base.s.ptr = freep = prevp = &base;
		base.s.size = 0;
	}
	for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
		if (p->s.size >= n_units) {
			if (p->s.size == n_units) {
				prevp->s.ptr = p->s.ptr;
			}
			else {
				p->s.size -= n_units;
				p += p->s.size;
				p->s.size = n_units;
			}
			freep = prevp;
			return (void*) (p + 1);
		}
		if (p == freep) {
			if((p = b_morecore(n_units)) == NULL) {
				return NULL;
			}
		}
	}
}

/*
 * Same as b_malloc but zores the memory before returning it.
 */

void *b_zmalloc(size_t n_bytes) {
	void* memory = b_malloc(n_bytes);
	if (memory != NULL) {
		memset(memory, 0, n_bytes);
		return memory;
	}
	return NULL;
}
