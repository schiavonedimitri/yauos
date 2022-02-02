#include <stddef.h>
#include <stdint.h>
#include <arch/paging.h>
#include <arch/kernel/vm.h>
#include <arch/mmu.h>
#include <arch/types.h>
#include <kernel/printk.h>


/* 
 * This routine gets the virtual address of the page table containing the mapping for a virtual address.
 * It works by using the well known recursive directory trick. The last entry of the kernel directory (entry 1023)
 * is mapped to the physical address of the kernel_directory itself. This implies that the virtual address space from
 * 0xFFC00000 to 0xFFFFFFFF can be used to access any kernel_directory entry, aka page tables.
 */
static virt_addr_t get_table_virtual_address(virt_addr_t addr) {
    // First get the directory index from the virtual address.
    size_t dir_entry_index = addr >> 22 & 0x3FF;
    // Add the directory index multiplied by the size of a page to the base address of the recursive directory address space.
    return (virt_addr_t) RECURSIVE_DIRECTORY_START_REGION + (dir_entry_index * PAGE_SIZE);
}