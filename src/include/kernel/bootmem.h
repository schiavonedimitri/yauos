#ifndef BOOTMEM_H
#define BOOTMEM_H

#include <stddef.h>
#include <stdint.h>

void *b_malloc(size_t);
void *b_zmalloc(size_t);
void b_free(void*);

#endif /** BOOTMEM_H */
