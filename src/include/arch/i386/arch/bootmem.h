#ifndef BOOTMEM_H
#define BOOTMEM_H

#include <stdint.h>

void *bmalloc(size_t);
void bfree(void*);

#endif /** BOOTMEM_H */
