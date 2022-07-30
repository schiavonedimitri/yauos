#ifndef ALIGN_H
        #define ALIGN_H

        #include <stdint.h>
        #include <arch/mmu.h>
        #include <arch/types.h>

        #define ALIGN(address, bytes) (typeof(address)) (((uintptr_t) (address) + bytes - 1) & ~(bytes - 1))
        #define IS_ALIGNED(address, bytes) (typeof(address)) ((uintptr_t) (address) % bytes == 0)
        #define PAGE_ROUND_UP(address) (typeof(address)) (((uintptr_t) (address) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
        #define PAGE_ROUND_DOWN(address) (typeof(address)) (((uintptr_t) (address)) & ~(PAGE_SIZE - 1))

#endif /** ALIGN_H */
