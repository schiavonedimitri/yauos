#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>
#include <arch/types.h>

typedef struct cpu_data {
    uint8_t lapic_id;
    bool bsp;
    struct cpu_data *cpu_data;
} cpu_data_t;

extern cpu_data_t *cpu_data;
extern uint8_t num_cpus;
extern phys_addr_t local_apic_address;

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

#endif /** CPU_H */