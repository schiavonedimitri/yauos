#ifndef _PMM_H
#define _PMM_H

#include <stdbool.h>
#include <stdint.h>
#include <arch/mmu.h>
#include <arch/types.h>

#define BLOCK_SIZE PAGE_SIZE

extern virt_addr_t _KERNEL_START_;
extern virt_addr_t _KERNEL_END_;
static void set_region(phys_addr_t, size_t, bool);

#define reserve_region(addr, size) set_region(addr, size, 1);
#define free_region(addr, size) set_region(addr, size, 0);

#endif /** _PMM_H */
