#ifndef MMU_H
#define MMU_H

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <arch/types.h>

#endif /** __ASSEMBLER__ */

#define KERNEL_PHYSICAL_BASE 0x100000
#define KERNEL_VIRTUAL_BASE 0xC0100000
#define PAGE_SIZE 4096

/*
 * For use in assembly code.
 */

#ifdef __ASSEMBLER__

#define VIRTUAL_TO_PHYSICAL(address) (address - (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE))
#define PHYSICAL_TO_VIRTUAL(address) (address + (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE))

#else

/*
 * For use in c code.
 */

#define VIRTUAL_TO_PHYSICAL(address) ((phys_addr_t) (((uintptr_t) address) - (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE)))
#define PHYSICAL_TO_VIRTUAL(address) ((virt_addr_t) (((uintptr_t) address) + (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE)))

#endif /** __ASSEMBLER__ */

#endif /** MMU_H */
