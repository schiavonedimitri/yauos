#ifndef CPU_H
        #define CPU_H

        #ifndef __ASSEMBLER__

                #include <stdbool.h>
                #include <stddef.h>
                #include <stdint.h>
                #include <arch/types.h>

        #endif /** __ASSEMBLER__ */

        /*
        * EFLAGS register definitions.
        */

        #define EFLAGS_CARRY_UNSET 1
        #define EFLAGS_CARRY_SET 0
        #define EFLAGS_CARRY_SHIFT 0
        #define EFLAGS_PARITY_UNSET 0
        #define EFLAGS_PARITY_SET (1 << 2)
        #define EFLAGS_PARITY_SHIFT 2
        #define EFLAGS_AUXILIARY_FLAG_UNSET 0
        #define EFLAGS_AUXILIARY_FLAGS_SET (1 << 4)
        #define EFLAGS_AUXILIARY_FLAG_SHIFT 4
        #define EFLAGS_ZERO_FLAG_UNSET 0
        #define EFLAGS_ZERO_FLAG_SET (1 << 6)
        #define EFLAGS_ZERO_FLAG_SHIFT 6
        #define EFLAGS_SIGN_FLAG_UNSET 0
        #define EFLAGS_SIGN_FLAG_SET (1 << 7)
        #define EFLAGS_SIGN_FLAG_SHIFT 7
        #define EFLAGS_TRAP_FLAG_UNSET 0
        #define EFLAGS_TRAP_FLAG_SET (1 << 8)
        #define EFLAGS_TRAP_FLAG_SHIFT 8
        #define EFLAGS_INTERRUPT_ENABLE_FLAG_UNSET 0
        #define EFLAGS_INTERRUPT_ENABLE_FLAG_SET (1 << 9)
        #define EFLAGS_INTERRUPT_ENABLE_FLAG_SHIFT 9
        #define EFLAGS_DIRECTION_FLAG_UNSET 0
        #define EFLAGS_DIRECTION_FLAG_SET (1 << 10)
        #define EFLAGS_DIRECTION_FLAG_SHIFT 10
        #define EFLAGS_OVERFLOW_FLAG_UNSET 0
        #define EFLAGS_OVERFLOW_FLAG_SET (1 << 11)
        #define EFLAGS_OVERFLOW_FLAG_SHIFT 11
        #define EFLAGS_IO_PRIVILEGE_LEVEL_0 0 
        #define EFLAGS_IO_PRIVILEGE_LEVEL_1 1
        #define EFLAGS_IO_PRIVILEGE_LEVEL_2 2
        #define EFLAGS_IO_PRIVILEGE_LEVEL_3 3
        #define EFLAGS_IO_PRIVILEGE_LEVEL_SHIFT 13
        #define EFLAGS_NESTED_TASK_FLAG_UNSET 0
        #define EFLAGS_NESTED_TASK_FLAG_SET (1 << 14)
        #define EFLAGS_NESTED_TASK_FLAG_SHIFT 14
        #define EFLAGS_RESUME_FLAG_UNSET 0
        #define EFLAGS_RESUME_FLAG_SET (1 << 16)
        #define EFLAGS_RESUME_FLAG_SHIFT 16
        #define EFLAGS_VIRTUAL_8086_MODE_FLAG_UNSET 0
        #define EFLAGS_VIRTUAL_8086_MODE_FLAG_SET (1 << 17)
        #define EFLAGS_VIRTUAL_8086_MODE_FLAG_SHIFT 17
        #define EFLAGS_ALIGNMENT_CHECK_FLAG_UNSET 0
        #define EFLAGS_ALIGNMENT_CHECK_FLAG_SET (1 << 18)
        #define EFLAGS_ALIGNMENT_CHECK_FLAG_SHIFT 18
        #define EFLAGS_VIRTUAL_INTERRUPT_FLAG_UNSET 0
        #define EFLAGS_VIRTUAL_INTERRUPT_FLAG_SET (1 << 19)
        #define EFLAGS_VIRTUAL_INTERRUPT_FLAG_SHIFT 19
        #define EFLAGS_VIRTUAL_INTERRUPT_PENDING_FLAG_UNSET 0
        #define EFLAGS_VIRTUAL_INTERRUPT_PENDING_FLAG_SET (1 << 20)
        #define EFLAGS_VIRTUAL_INTERRUPT_PENDING_FLAG_SHIFT 20
        #define EFLAGS_CPUID_NOT_PRESENT 0
        #define EFLAGS_CPUID_PRESENT (1 << 21)
        #define EFLAGS_CPUID_SHIFT 21


        /*
        * CR0 register definitions.
        */

        #define CR0_PROTECTED_MODE_ENABLED 1
        #define CR0_PROTECTED_MODE_DISABLED 0
        #define CR0_PROTECTED_MODE_SHIFT 0
        #define CR0_MONITOR_CO_PROCESSOR 0
        #define CR0_DONT_MONITOR_CO_PROCESSOR (1 << 1)
        #define CR0_MONITOR_CO_PROCESSOR_SHIFT 1
        #define CR0_X87_FPU_PRESENT 0
        #define CR0_X87_FPU_EMULATED (1 << 2)
        #define CR0_X87_FPU_SHIFT 2
        #define CR0_TASK_SWITCHED 0
        #define CR0_TASK_NOT_SWITCHED (1 << 3)
        #define CR0_TASK_SHIFT 3
        #define CR0_FPU_TYPE_80287 0
        #define CR0_FPU_TYPE_80387 (1 << 4)
        #define CR0_FPU_TYPE_SHIFT 4
        #define CR0_X87_FPU_ERROR_REPORTING_ENABLED 0
        #define CR0_X87_FPU_ERROR_REPORTING_DISABLED (1 << 5)
        #define CR0_X87_FPU_ERROR_REPORTING_SHIFT 5
        #define CR0_WRITE_PROTECT_ENABLED (1 << 16)
        #define CR0_WRITE_PROTECT_DISABLED 0
        #define CR0_WRITE_PROTECT_SHIFT 16
        #define CR0_ALIGNMENT_CHECKS_ENABLED (1 << 18)
        #define CR0_ALIGNMENT_CHECKS_DISABLED 0
        #define CR0_ALIGNMENT_CHECKS_SHIFT 18
        #define CR0_WRITE_THROUGH_CACHING_ENABLED 0
        #define CR0_WRITE_THROUGH_CACHING_DISABLED (1 << 29)
        #define CR0_WRITE_THROUGH_CACHING_SHIFT 29
        #define CR0_MEMORY_CACHE_ENABLED 0
        #define CR0_MEMORY_CACHE_DISABLED (1 << 30)
        #define CR0_MEMORY_CACHE_SHIFT 30
        #define CR0_PAGING_ENABLED (1 << 31)
        #define CR0_PAGING_DISABLED 0
        #define CR0_PAGING_SHIFT 31

        #ifndef __ASSEMBLER__

                typedef struct cpu_data {
                        uint8_t lapic_id;
                        bool bsp;
                        virt_addr_t *gdt;
                        struct cpu_data *cpu;
                } cpu_data_t;

                extern cpu_data_t *cpu_data;
                extern cpu_data_t *cpu asm("%gs:0");
                extern void* cpu_data_2 asm("%gs:4");

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

                static inline uint32_t read_cr0(void) {
                        uint32_t cr0;
                        asm volatile("movl %%cr0, %0;" : "=r" (cr0));
                        return cr0;
                }

                static inline void write_cr0(uint32_t cr0) {
                        asm volatile("movl %0, %%cr0;" : : "r" (cr0));
                }

                static inline uint32_t read_cr2(void) {
                        virt_addr_t cr2;
                        asm volatile("movl %%cr2, %0;" : "=r" (cr2));
                        return cr2;
                }

                static inline void arch_halt(void) {
                        asm volatile("hlt");
                }

                static inline void arch_cli(void) {
                        asm volatile("cli");
                }

                static inline void arch_sti(void) {
                        asm volatile("sti");
                }

                static inline uint32_t arch_atomic_swap(uint32_t new_value, volatile uint32_t *lock) {
                        return _xchg(new_value, lock);
                }
                
        #endif /** __ASSEMBLER__ */

#endif /** CPU_H */