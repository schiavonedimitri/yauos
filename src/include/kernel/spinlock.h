#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>
#include <arch/cpu/cpu.h>

typedef volatile uint32_t spinlock_t;
 
void lock(spinlock_t *lock);
void unlock(spinlock_t* lock);

#endif /** SPINLOCK_H */