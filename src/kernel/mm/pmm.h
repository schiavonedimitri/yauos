#ifndef _PMM_H
#define _PMM_H

#include <stdint.h>
#include <arch/mmu.h>
#include <arch/types.h>

#define BLOCK_SIZE PAGE_SIZE

extern virt_addr_t _KERNEL_START_;
extern virt_addr_t _KERNEL_END_;

#endif /** _PMM_H */
