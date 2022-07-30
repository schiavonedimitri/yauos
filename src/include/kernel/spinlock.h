#ifndef SPINLOCK_H
        #define SPINLOCK_H

        #include <stdbool.h>
        #include <stdint.h>
        #include <arch/cpu/cpu.h>

        typedef struct spinlock {
        char name[8];
        volatile uint32_t lock;
        } spinlock_t;
        
        void lock(spinlock_t *lock);
        void unlock(spinlock_t* lock);

#endif /** SPINLOCK_H */