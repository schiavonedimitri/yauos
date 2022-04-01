#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>
#include <arch/cpu/cpu.h>

// Declared in arch_main.c
extern bool smp;

typedef volatile uint32_t spinlock_t;
 
inline static void lock(void) {
	cli();
}

inline static void unlock(void) {
	sti();
}

inline static void lock_smp(spinlock_t *mutex) {
	while(xchg(1, mutex) != 0);
}
 
inline static void unlock_smp(spinlock_t *mutex){
	*mutex = 0;
}

#endif /** SPINLOCK_H */