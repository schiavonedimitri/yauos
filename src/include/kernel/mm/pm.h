#ifndef PM_H
        #define PM_H

        #include <stdbool.h>
        #include <stdint.h>
        #include <arch/mmu.h>
        #include <arch/types.h>

        extern virt_addr_t kernel_virtual_end;

        #define BLOCK_SIZE PAGE_SIZE

        typedef struct bitmap_list {
                uint32_t *bitmap;
                size_t bitmap_size;
                phys_addr_t first_addr;
                phys_addr_t last_addr;
                size_t total_blocks;
                size_t reserved_blocks;
                size_t used_blocks;
                struct bitmap_list *next;
        } bitmap_list_t;

        /*
        * Exported by the linker script.
        */

        extern virt_addr_t _KERNEL_START_;
        extern virt_addr_t _KERNEL_END_;

        phys_addr_t get_free_frame(void);
        void free_frame(phys_addr_t);

#endif /** PM_H */