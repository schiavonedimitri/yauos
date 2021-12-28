#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <arch/bootmem.h>
#include <arch/defs.h>

/*
 * Used by morecore() to request memory chunks. By default it requests 1024Kb chunks per call.
 * Shouldn't be higher than EARLY_HEAP_SIZE / sizeof(Header) or allocations will fail due to the pool being limited to EARLY_HEAP_SIZE bytes.
 */
#define MORECORE_DEFAULT 128

/*
 * This an implementation of the K&R malloc for use during early boot. Although a heap early in the boot is overkill it is indeed convenient.
 * Programming without dynamic memory is always more convoluted and the rationale for this is the following:
 * No assumptions can be made about memory (not even memory past the end of the kernel) during boot before inspecting
 * a memory map provided by firmware or by a predecessor in the boot chain.
 * Due to this the allocator draws from a pool of EARLY_HEAP_SIZE reserved in the kernel .bss section which is
 * guarenteed instead (because this is an elf kernel, if not enough memory was available for the loader it wouldn't have loaded us).
 * This is mostly used during early stage in the initialization process, before the memory manager is up and running and for aiding at bootstrapping
 * the memory manager itself. A simple watermark allocator could've been used since this memory will be eventually recycled by the real memory manager.
 * Nevertheless the convenience of having a free and since the allocator manages such a small memory pool should add little to no overhead for great added functionality.
 */

extern uintptr_t bootmem_start;
extern uintptr_t bootmem_end;

typedef long Align;

typedef union header {
	struct {
	union header *ptr;
	size_t size;
	} s;
	Align x;
} Header;

static void *memory_end = &bootmem_end;
static void *free_mem_ptr = &bootmem_start;
static Header base;
static Header *freep;

static void* balloc(size_t bytes) {
	if (!IS_ALIGNED(free_mem_ptr, alignof(Align))) {
		free_mem_ptr = ALIGN(free_mem_ptr, alignof(Align));
	}
	if ((uint32_t) free_mem_ptr + bytes > (uint32_t) memory_end) {
		return (void*) -1;
	}
	void *mem_ptr = free_mem_ptr;
	free_mem_ptr = (void*) (uint32_t) free_mem_ptr + bytes;
	return mem_ptr;
}

void bfree(void *ap) {
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

static Header* morecore(size_t n_units) {
	char *p;
	Header *hp;
	if (n_units < MORECORE_DEFAULT) {
		n_units = MORECORE_DEFAULT;
	}
	p = balloc(n_units * sizeof(Header));
	if (p == (char*) -1) {
		return 0;
	}
	hp = (Header*) p;
	hp->s.size = n_units;
	bfree((void*) (hp + 1));
	return freep;
}

void* bmalloc(size_t n_bytes) {
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
			return (void*)(p + 1);
		}
		if (p == freep) {
			if((p = morecore(n_units)) == 0) {
				return NULL;
			}
		}
	}
}
