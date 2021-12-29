#ifndef BOOTMEM_H
#define BOOTMEM_H
#include <stddef.h>
#include <arch/types.h>

virt_addr_t *bmalloc(size_t bytes);
void bfree(virt_addr_t *ptr);

#endif /** BOOTMEM_H */
