#ifndef _PM_H
#define _PM_H

#include <stdbool.h>
#include <stdint.h>
#include <arch/mmu.h>
#include <arch/types.h>

#define BLOCK_SIZE PAGE_SIZE

typedef struct bitmap_list {
    uint32_t *bitmap;
    size_t bitmap_size;
    phys_addr_t first_addr;
    phys_addr_t last_addr;
    size_t total_blocks;
    size_t reserved_blocks;
    size_t used_blocks;
    struct bitmap_list *next;
} bitmap_list_t;

extern virt_addr_t _KERNEL_START_;
extern virt_addr_t _KERNEL_END_;
static int reserve_region(phys_addr_t, size_t);

#endif /** _PM_H */
