#ifndef MMU_H
#define MMU_H

#ifndef EARLY_HEAP_SIZE
#define EARLY_HEAP_SIZE 4096
#endif

#define KERNEL_PHYSICAL_BASE 0x100000
#define KERNEL_VIRTUAL_BASE 0xC0100000
#define PAGE_SIZE 4096


#ifdef __ASSEMBLER__

#define VIRTUAL_TO_PHYSICAL(x) (x - (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE))
#define PHYSICAL_TO_VIRTUAL(x) (x + (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE))

#else
#include <arch/types.h>

#define VIRTUAL_TO_PHYSICAL(x) ((phys_addr_t*) (((uint32_t) x) - (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE)))
#define PHYSICAL_TO_VIRTUAL(x) ((virt_addr_t*) (((uint32_t) x) + (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE)))

#endif /** __ASSEMBLER__ */

#endif /** MMU_H */
