#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <kernel/assert.h>
#include <arch/kernel/mm/vm.h>
#include <arch/types.h>
#include <kernel/mm/pm.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>
#include <lib/string.h>

static virt_addr_t heap_start = (virt_addr_t) &_KERNEL_END_;
static virt_addr_t heap_end;
static virt_addr_t heap_brk;
static Header base;
static Header *freep;

ssize_t kmalloc_init() {
    bool failed = false;
    size_t failed_idx = 0;
    virt_addr_t tmp = heap_start;
    for (size_t i = 0; i < MAX_HEAP_SIZE / PAGE_SIZE; i++) {
        phys_addr_t frame = get_free_frame();
        if (frame != (phys_addr_t) -1) {
            if (map_page(frame, tmp, PROT_KERN | PROT_READ_WRITE)) {
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
        virt_addr_t tmp2 = heap_start;
        for (size_t i = 0; i < failed_idx; i++) {
            unmap_page(tmp2);
            tmp2 += PAGE_SIZE;
        }
        return -1;
    }
    heap_end = heap_start + MAX_HEAP_SIZE;
    heap_brk = heap_start;
	printk("[KERNEL]: heap start: %x\n[KERNEL]: heap end: %x\n", heap_start, heap_end);
    return 1;
}

static void* sbrk(size_t size) {
    if (heap_brk + size >= heap_end) {
        return NULL;
    }
    else {
        void* ret = (void*) heap_brk;
        heap_brk += size;
        return ret; 
    }
}

void kfree(void *ap) {
	Header *bp, *p;
	bp = (Header*) ap - 1;
	memset(ap, 0, bp->s.size * sizeof(Header));
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
	void *p;
	Header *hp;
	if (n_units < MORECORE_DEFAULT) {
		n_units = MORECORE_DEFAULT;
	}
	p = sbrk(n_units * sizeof(Header));
	if (p == NULL) {
		return NULL;
	}
	hp = (Header*) p;
	hp->s.size = n_units;
	kfree((void*) (hp + 1));
	return freep;
}

/*
 * Memory allocated is zeroed before being returned to the user.
 */

void* kmalloc(size_t n_bytes) {
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
			memset((p + 1), 0, n_bytes);
			return (void*) (p + 1);
		}
		if (p == freep) {
			if((p = morecore(n_units)) == NULL) {
				return NULL;
			}
		}
	}
}