#include <arch/cpu/cpu.h>
#include <arch/cpu/smp.h>
#include <kernel/spinlock.h>

inline void lock(spinlock_t *lock) {
        if (!smp) {
                arch_cli();
        }
        else {
                while(arch_atomic_swap(1, &(lock->lock)) != 0);
        }
}

inline void unlock(spinlock_t *lock) {
        if (!smp) {
                arch_sti();
        }
        else {
                lock->lock = 0;
        }
}