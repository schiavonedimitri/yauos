#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/kernel/mm/vm.h>
#include <arch/types.h>
#include <kernel/assert.h>
#include <kernel/mm/pm.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>
#include <lib/string.h>

/*
 * As with the bootmem allocator, this is just another K&R allocator that is backed up by the physical memory manager and the virtual memory manager.
 * This is used for kernel internal use. In the future a buddy allocator and a slab allocator will be added when the other essential structures of the kernel
 * will be designed and when userspace comes into play.
 */

static virt_addr_t heap_brk;
static virt_addr_t heap_end;
static Header base;
static Header *freep;

/*
 * Initializes the kernel heap system.
 * For now the heap is a zone of MAX_HEAP_SIZE starting at the first address aligned to max_align_t after the kernel end. 
 * This initialization code pre-allocates MAX_HEAP_SIZE from the physical memory manager and maps it to the kernel 
 * virtual address space starting at _KERNEL_END_.
 * Returns 0 on success or -1 in case of failure.
 */

ssize_t k_malloc_init() {
	heap_brk = PAGE_ROUND_UP(kernel_virtual_end);
	heap_end = heap_brk + KERNEL_HEAP_SIZE;
	//panic("heap break: %x\nheap end: %x\n", heap_brk, heap_end);
    bool failed = false;
    size_t failed_idx = 0;
    virt_addr_t tmp = heap_brk;
    for (size_t i = 0; i < KERNEL_HEAP_SIZE / PAGE_SIZE; i++) {
        phys_addr_t frame = get_free_frame();
        if (frame != (phys_addr_t) -1) {
            if (map_page(frame, tmp, PROT_PRESENT | PROT_KERN | PROT_READ_WRITE)) {
                tmp += PAGE_SIZE;
                continue;
            }
            else {
                failed = true;
                failed_idx = i;
                break;
            }
        }
        else {
            failed = true;
            failed_idx = i;
        }
    }
    if (failed) {
        virt_addr_t tmp2 = heap_brk;
        for (size_t i = 0; i < failed_idx; i++) {
            unmap_page(tmp2);
            tmp2 += PAGE_SIZE;
        }
        return -1;
    }
	printk("[KERNEL]: Heap initialized.\n[KERNEL]: Heap start: %x\n[KERNEL]: Heap end: %x\n[KERNEL]: Heap size: %dMb\n", heap_brk, heap_end, (heap_end - heap_brk) / (1024 * 1024));
    return 0;
}

/*
 * Increases the heap break by the requested amount aligned to max_align_t.
 * Returns -1 in case of failure.
 */

static void* k_sbrk(size_t amount) {
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

void k_free(void *ap) {
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

/*
 * Requests more memory from the kernel heap pool.
 */

static Header* k_morecore(size_t n_units) {
	void *p;
	Header *hp;
	if (n_units < MORECORE_DEFAULT) {
		n_units = MORECORE_DEFAULT;
	}
	p = k_sbrk(n_units * sizeof(Header));
	if (p == (void*) -1) {
		return NULL;
	}
	hp = (Header*) p;
	hp->s.size = n_units;
	k_free((void*) (hp + 1));
	return freep;
}

/*
 * Returns the starting address of the requested memory.
 * In case of failure NULL is returned.
 */

void* k_malloc(size_t n_bytes) {
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
			if((p = k_morecore(n_units)) == NULL) {
				return NULL;
			}
		}
	}
}

/*
 * Same as k_malloc but zores the memory before returning it.
 */

void *k_zmalloc(size_t n_bytes) {
	void* memory = k_malloc(n_bytes);
	if (memory != NULL) {
		memset(memory, 0, n_bytes);
		return memory;
	}
	return NULL;
}