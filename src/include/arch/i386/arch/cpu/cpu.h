#ifndef CPU_H
#define CPU_H

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/types.h>

#endif /** __ASSEMBLER__ */

#define CR0_PROTECTED_MODE_ENABLED 0x1
#define CR0_PROTECTED_MODE_DISABLED 0x0
#define CR0_PROTECTED_MODE_SHIFT 0
#define CR0_MONITOR_CO_PROCESSOR 0x0
#define CR0_DONT_MONITOR_CO_PROCESSOR (1 << 1)
#define CR0_MONITOR_CO_PROCESSOR_SHIFT 1
#define CR0_X87_FPU_PRESENT 0x0
#define CR0_X87_FPU_EMULATED (1 << 2)
#define CR0_X87_FPU_SHIFT 2
#define CR0_TASK_SWITCHED 0x0
#define CR0_TASK_NOT_SWITCHED (1 << 3)
#define CR0_TASK_SHIFT 3
#define CR0_FPU_TYPE_80287 0x0
#define CR0_FPU_TYPE_80387 (1 << 4)
#define CR0_FPU_TYPE_SHIFT 4
#define CR0_X87_FPU_ERROR_REPORTING_ENABLED 0x0
#define CR0_X87_FPU_ERROR_REPORTING_DISABLED (1 << 5)
#define CR0_X87_FPU_ERROR_REPORTING_SHIFT 5
#define CR0_WRITE_PROTECT_ENABLED (1 << 16)
#define CR0_WRITE_PROTECT_DISABLED 0x0
#define CR0_WRITE_PROTECT_SHIFT 16
#define CR0_ALIGNMENT_CHECKS_ENABLED (1 << 18)
#define CR0_ALIGNMENT_CHECKS_DISABLED 0x0
#define CR0_ALIGNMENT_CHECKS_SHIFT 18
#define CR0_WRITE_THROUGH_CACHING_ENABLED 0x0
#define CR0_WRITE_THROUGH_CACHING_DISABLED (1 << 29)
#define CR0_WRITE_THROUGH_CACHING_SHIFT 29
#define CR0_MEMORY_CACHE_ENABLED 0x0
#define CR0_MEMORY_CACHE_DISABLED (1 << 30)
#define CR0_MEMORY_CACHE_SHIFT 30
#define CR0_PAGING_ENABLED (1 << 31)
#define CR0_PAGING_DISABLED 0x0
#define CR0_PAGING_SHIFT 31

#ifndef __ASSEMBLER__

typedef struct cpu_data {
    uint8_t lapic_id;
    bool bsp;
    virt_addr_t *gdt;
    phys_addr_t current_directory;
    struct cpu_data *cpu;
} cpu_data_t;

extern cpu_data_t *cpu_data;
extern cpu_data_t *cpu asm("%gs:0"); 

// xchg.S

extern uint32_t _xchg(uint32_t, volatile uint32_t*);

static inline void load_gs(uint16_t gs) {
    asm volatile("movw %0, %%gs" : : "r" (gs));
}

static inline uint32_t read_eflags(void) {
    uint32_t eflags;
    asm volatile("pushfl; popl %0" : "=r" (eflags));
    return eflags;
}

static inline uint32_t read_cr2(void) {
    uint32_t cr2;
    asm volatile("movl %%cr2, %0;" : "=r" (cr2));
    return cr2;
}

static inline void halt(void) {
    asm volatile("hlt");
}

static inline void cli(void) {
    asm volatile("cli");
}

static inline void sti(void) {
    asm volatile("sti");
}

static inline uint32_t xchg(uint32_t new_value, volatile uint32_t *lock) {
    return _xchg(new_value, lock);
}

#endif /** __ASSEMBLER__ */

#endif /** CPU_H */