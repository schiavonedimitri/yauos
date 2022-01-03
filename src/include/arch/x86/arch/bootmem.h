#ifndef BOOTMEM_H
#define BOOTMEM_H

#include <stdint.h>

void *bmalloc(size_t bytes);
void bfree(void *ptr);

#endif /** BOOTMEM_H */
