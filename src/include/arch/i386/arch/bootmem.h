#ifndef BOOTMEM_H
#define BOOTMEM_H

void *bmalloc(size_t bytes);
void bfree(void *ptr);

#endif /** BOOTMEM_H */
