#ifndef _EXCEPTION_INTERRUPT_H
#define _EXCEPTION_INTERRUPT_H

#include <stdint.h>

struct context {
   uint32_t ds;
   uint32_t edi, esi, ebp, ignore, ebx, edx, ecx, eax;
   uint32_t number, error_code;
   uint32_t eip, cs, eflags, esp, ss;
} __attribute__((packed));

typedef struct context exception_context_t;
typedef struct context interrupt_context_t;

#endif /** _EXCEPTION_INTERRUPT_H */