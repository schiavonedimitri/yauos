#ifndef _INTERRUPT_H
#define _INTERRUPT_H

struct interrupt_context {
   uint32_t ds;
   uint32_t edi, esi, ebp, ignore, ebx, edx, ecx, eax;
   uint32_t interrupt_number, error_code;
   uint32_t eip, cs, eflags, esp, ss;
} __attribute__((packed));

typedef struct interrupt_context interrupt_context_t;

#endif /** _INTERRUPT_H */