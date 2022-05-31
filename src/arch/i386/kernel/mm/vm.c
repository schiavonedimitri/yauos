#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/paging.h>
#include <arch/kernel/mm/vm.h>
#include <arch/mmu.h>
#include <arch/types.h>
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

ssize_t map_page(phys_addr_t phys, virt_addr_t virt, uint16_t flags) {
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
    }
    // Get the address of the page table virt refers to.
    page_table_t *table = (page_table_t*) get_table_virtual_address(virt);
    table->entry[tbl_idx].address = phys >> 12;
    table->entry[tbl_idx].present = flags & 0x1;
    table->entry[tbl_idx].read_write = flags >> 1 & 0x1;
    table->entry[tbl_idx].user_supervisor = flags >> 2 & 0x1;
    table->entry[tbl_idx].page_write_through = flags >> 3 & 0x1;
    table->entry[tbl_idx].page_cache_disable = flags >> 4 & 0x1;
    table->entry[tbl_idx].global = flags >> 8 & 0x1;
    flush_tlb_single(virt);
    return 1;
}

void unmap_page(virt_addr_t address) {
    bool unmap_directory = true;
    size_t dir_idx = address >> 22 & 0x3FF;
    size_t tbl_idx = address >> 12 & 0x3FF;
    page_table_t *table = (page_table_t*) get_table_virtual_address(address);
    table->entry[tbl_idx].present = 0;
    free_frame(table->entry[tbl_idx].address << 12);
    for (size_t i = 0; i < 1024; i++) {
        if (table->entry[i].present) {
            unmap_directory = false;
            break;
        }
    }
    if (unmap_directory) {
        kernel_directory.entry[dir_idx].present = 0;
        free_frame(kernel_directory.entry[dir_idx].address << 12);
    }
    return;
}