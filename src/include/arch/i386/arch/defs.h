#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <arch/mmu.h>
#include <arch/types.h>

#define ALIGN(address, bytes) (typeof(address)) (((uintptr_t) (address) + bytes - 1) & ~(bytes - 1))
#define IS_ALIGNED(address, bytes) (typeof(address)) ((uintptr_t) (address) % bytes == 0)
#define PAGE_ROUND_UP(address) (typeof(address)) (((uintptr_t) (address) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_ROUND_DOWN(address) (typeof(address)) (((uintpr_t) (address)) & ~(PAGE_SIZE - 1))
#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef DEBUG
#define assert(expression) ((expression) ? (void) 0 : panic("Assertion failed: \"" #expression "\" file: %s function: %s line: %d\n", __FILENAME__, __func__, __LINE__))
#else
#define assert(expression)
#endif

#endif /** DEFS_H */
