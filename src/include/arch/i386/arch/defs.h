#ifndef DEFS_H
#define DEFS_H

#define ALIGN(address, bytes) (void*) (((uint32_t) (address) + bytes - 1) & ~(bytes - 1))
#define IS_ALIGNED(address, bytes) ((uint32_t) (address) % bytes == 0)
#define PAGE_ROUND_UP(address) (void*) (((uint32_t) (address) + 4096 - 1) & ~(4096 - 1))
#define PAGE_ROUND_DOWN(address) (void*) (((uint32_t) (address)) & ~(4096 - 1))

#endif /** DEFS_H */
