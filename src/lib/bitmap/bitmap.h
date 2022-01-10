#ifndef _BITMAP_H
#define _BITMAP_H

#if ARCH_BITS == 32

#define BITMAP_BITS 32
#define BITMAP_FULL_MASK 0xFFFFFFFF

#elif ARCH_BITS == 64

#define BITMAP_BITS 64
#define BITMAP_FULL_MASK 0xFFFFFFFFFFFFFFFF

#endif

#endif /** _BITMAP_H */
