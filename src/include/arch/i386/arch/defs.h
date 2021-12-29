#ifndef DEFS_H
#define DEFS_H

#define ALIGN(address, bytes) (void*) (((uint32_t) (address) + bytes - 1) & ~(bytes - 1))
#define IS_ALIGNED(address, bytes) ((uint32_t) (address) % bytes == 0)
#define PAGE_ROUND_UP(address) (void*) (((uint32_t) (address) + 4096 - 1) & ~(4096 - 1))
#define PAGE_ROUND_DOWN(address) (void*) (((uint32_t) (address)) & ~(4096 - 1))
#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef DEBUG
#define assert(expression) ((expression) ? (void) 0 : panic("Assertion failed: \"" #expression "\" file: %s function: %s line: %d\n", __FILENAME__, __func__, __LINE__))
#else
#define assert(expression)
#endif

#endif /** DEFS_H */
