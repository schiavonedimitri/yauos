#include <arch/cpu/cpu.h>
#include <kernel/spinlock.h>

inline void lock(spinlock_t *lock) {
    if (!smp) {
        cli();
    }
    else {
        while(xchg(1, lock) != 0);
    }
}

inline void unlock(spinlock_t *lock) {
    if (!smp) {
        sti();
    }
    else {
        *lock = 0;
    }
}