#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/paging.h>
#include <arch/kernel/mm/vm.h>
#include <arch/mmu.h>
#include <arch/types.h>
#include <kernel/assert.h>
#include <kernel/mm/pm.h>

/* 
 * This routine gets the virtual address of the page table containing the mapping for a virtual address.
 * It works by using the well known recursive directory trick. The last entry of the kernel directory (entry 1023)
 * is mapped to the physical address of the kernel_directory itself. This implies that the virtual address space from
 * 0xFFC00000 to 0xFFFFFFFF can be used to access any kernel_directory entry, aka page tables.
 */

inline static virt_addr_t get_table_virtual_address(virt_addr_t addr) {
        
        // First get the directory index from the virtual address.
    
        size_t dir_entry_index = addr >> 22 & 0x3FF;
    
        // Add the directory index multiplied by the size of a page to the base address of the recursive directory address space.
    
        return (virt_addr_t) RECURSIVE_DIRECTORY_START_REGION + (dir_entry_index * PAGE_SIZE);
}

int map_page(phys_addr_t phys, virt_addr_t virt, uint16_t flags, bool kmalloc_init) {
        bool dir_created = false;
        size_t dir_idx = virt >> 22 & 0x3FF;
        size_t tbl_idx = virt >> 12 & 0x3FF;

        // If this page table does not exist, create it.
        
        if (!kernel_directory.entry[dir_idx].present) {
                
                // Allocate a new frame for the new page table.
                
                phys_addr_t frame = get_free_frame();
                if (frame == (phys_addr_t) -1) {
                        return -1;
                }
                kernel_directory.entry[dir_idx].address = frame >> 12;
                kernel_directory.entry[dir_idx].present = flags & 0x1;
                kernel_directory.entry[dir_idx].read_write = flags >> 1 & 0x1;
                kernel_directory.entry[dir_idx].user_supervisor = flags >> 2 & 0x1;
                kernel_directory.entry[dir_idx].page_write_through = flags >> 3 & 0x1;
                kernel_directory.entry[dir_idx].page_cache_disable = flags >> 4 & 0x1;
                dir_created = true;
        }
        
        // Get the address of the page table virt refers to.
        
        page_table_t *table = (page_table_t*) get_table_virtual_address(virt);
        
        /*
        * Check that we are not trying to map a page to an address that has already a mapping.
        * If a directory was created it means that there were no mappings for this address.
        * kmalloc_init lets us skip the check for addresses that were already mapped during
        * bootstrap in boot.S.
        */

        if (!dir_created && !kmalloc_init && table->entry[tbl_idx].present == 1) {
                return -1;
        }
        table->entry[tbl_idx].address = phys >> 12;
        table->entry[tbl_idx].present = flags & 0x1;
        table->entry[tbl_idx].read_write = flags >> 1 & 0x1;
        table->entry[tbl_idx].user_supervisor = flags >> 2 & 0x1;
        table->entry[tbl_idx].page_write_through = flags >> 3 & 0x1;
        table->entry[tbl_idx].page_cache_disable = flags >> 4 & 0x1;
        table->entry[tbl_idx].global = flags >> 8 & 0x1;
        flush_tlb_single(virt);
        return 0;
}

int unmap_page(virt_addr_t address) {
        bool unmap_directory = true;
        size_t dir_idx = address >> 22 & 0x3FF;
        size_t tbl_idx = address >> 12 & 0x3FF;
        page_table_t *table = (page_table_t*) get_table_virtual_address(address);
        if (table->entry[tbl_idx].present == 0) {
                return -1;
        }
        table->entry[tbl_idx].present = 0;
        free_frame(table->entry[tbl_idx].address << 12);
        table->entry[tbl_idx].address = 0;
        
        // Scan the entire directory to see if we can free it.
        
        for (size_t i = 0; i < 1024; i++) {
                if (table->entry[i].present) {
                        unmap_directory = false;
                        break;
                }
        }
        if (unmap_directory) {
                if (kernel_directory.entry[dir_idx].present == 0) {
                        return -1;
                }
                kernel_directory.entry[dir_idx].present = 0;
                free_frame(kernel_directory.entry[dir_idx].address << 12);
                kernel_directory.entry[dir_idx].address = 0;
        }
        flush_tlb_single(address);
        return 0;
}

void do_page_fault(virt_addr_t fault_address) {
        panic("Page fault at address: %x\n", fault_address);
}