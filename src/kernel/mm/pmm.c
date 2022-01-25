#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/arch.h>
#include <arch/mmu.h>
#include <arch/types.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <lib/bitmap.h>
#include <lib/string.h>
#include "pmm.h"

static phys_addr_t k_start = VIRTUAL_TO_PHYSICAL(&_KERNEL_START_);
static phys_addr_t k_end = VIRTUAL_TO_PHYSICAL(&_KERNEL_END_);
static unsigned long *pmm_bitmap = (unsigned long*) -1;
static size_t pmm_size = 0;
static size_t pmm_total_blocks = 0;
static size_t pmm_total_usable_blocks = 0;
static size_t pmm_reserved_blocks = 0;
static size_t pmm_used_blocks = 0;

static void print_memory_map(bootinfo_t *boot_info) {
	printk("[KERNEL]: Final Memory map\n");
	for (size_t i = 0; i < boot_info->memory_map_entries; i++) {
		printk("[KERNEL]: Region: [%lx-%lx] ", boot_info->memory_map_entry[i].base_addr, boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1);
		if (boot_info->memory_map_entry[i].type == MEMORY_AVAILABLE) {
			printk("[AVAILABLE]\n");
		}
		else if (boot_info->memory_map_entry[i].type == MEMORY_RECLAIMABLE) {
			printk("[RECLAIMABLE]\n");
		}
		else if (boot_info->memory_map_entry[i].type == MEMORY_RESERVED) {
			printk("[RESERVED]\n");
		}
	}
}

static void set_region(phys_addr_t start_addr, size_t size, bool reserve) {
	size_t region_in_blocks;
	// Rounding address to the next page because we can only use whole frames.
	start_addr = PAGE_ROUND_UP(start_addr);
	if (size < BLOCK_SIZE) {
		region_in_blocks = 1;
	}
	else {
		region_in_blocks = size / BLOCK_SIZE;
	}
	size_t bit = start_addr / BLOCK_SIZE;
	for (size_t i = 0; i < region_in_blocks; i++) {
		if (reserve) {
			pmm_reserved_blocks++;
			pmm_used_blocks++;
			bitmap_set(pmm_bitmap, bit++);
		}
		else {
			pmm_total_usable_blocks++;
			bitmap_unset(pmm_bitmap, bit++);
		}
	}
}

/*
 * The physical memory manager is based on a bitmap for the entire address space to account for possible regions to allocate for hardware I/O.
 * The first step in the initialization of the system is finding the correct place in memory with enough space to hold the bitmap which does 
 * not overlap with the kernel itself. After that all memory is initially marked as in use and the provided memory map is scanned for available
 * memory regions. For every region in the memory map marked as available the corresponding bits in the bitmap are marked as free.
 * Finally, the bitmap region and the kernel regions themselves are marked as in use.
 * TODO: in the future the physical memory manager should handle hardware I/O regions as well.
 */

void pmm_init(bootinfo_t *boot_info) {
	print_memory_map(boot_info);
	pmm_total_blocks = boot_info->address_space_size / BLOCK_SIZE;
	if (boot_info->address_space_size % BLOCK_SIZE) {
		pmm_total_blocks++;
	}
	pmm_size = pmm_total_blocks / 8;
	void *ptr = (void*) -1;
	if (pmm_total_blocks % 8) {
		pmm_size++;
	}
	for (size_t i = 0; i < boot_info->memory_map_entries; i++) {
		ptr = (void*) -1;
		if(boot_info->memory_map_entry[i].length >= pmm_size && boot_info->memory_map_entry[i].type == MEMORY_AVAILABLE) {
			if (boot_info->memory_map_entry[i].base_addr != (phys_addr_t) k_start) {
				if (ALIGN((phys_addr_t) (boot_info->memory_map_entry[i].base_addr + pmm_size - 1), PAGE_SIZE) < k_start) {
					ptr = ALIGN((void*) (phys_addr_t) boot_info->memory_map_entry[i].base_addr, PAGE_SIZE);
					break;
				}
				else {
					if (ALIGN(((phys_addr_t) k_end + pmm_size - 1), PAGE_SIZE) <= (phys_addr_t) (boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1)) {
						ptr = ALIGN((void*) k_end, PAGE_SIZE);
						break;
					}
				}
			}
			else {
				if (ALIGN(((phys_addr_t) k_end + pmm_size - 1), PAGE_SIZE) <= (phys_addr_t) (boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1)) {
					ptr = ALIGN((void*) k_end, PAGE_SIZE);
					break;
				}
			}
		}
	}
	if (ptr == (void*) -1) {
		panic("[KERNEL]: Failed to allocate memory for pmm bitmap!\n");
	}
	pmm_bitmap = (uintptr_t*) PHYSICAL_TO_VIRTUAL(ptr);
	// Mark all memory as used initially.
	memset(pmm_bitmap, 0xFF, pmm_size);
	// Free regions marked as free in the memory map.
	for (size_t i = 0; i < boot_info->memory_map_entries; i++) {
		if (boot_info->memory_map_entry[i].type == MEMORY_AVAILABLE) {
			free_region(boot_info->memory_map_entry[i].base_addr, boot_info->memory_map_entry[i].length);
		}
	}
	// Reserving kernel memory and bitmap memory because they were freed above, since they belong to the free memory regions.
	reserve_region(k_start, k_end - k_start);
	reserve_region(VIRTUAL_TO_PHYSICAL(pmm_bitmap), pmm_size);
	printk("[KERNEL]: Initialized physical memory\n[KERNEL]: Block size: %d bytes\n[KERNEL]: Usable blocks: %d\n[KERNEL]: Free blocks: %d\n[KERNEL]: Reserved blocks: %d\n[KERNEL]: Total usable memory: %dMb\n[KERNEL]: Total memory: %dMb\n", BLOCK_SIZE, pmm_total_usable_blocks, pmm_total_usable_blocks - pmm_reserved_blocks, pmm_reserved_blocks, (pmm_total_usable_blocks - pmm_reserved_blocks) * BLOCK_SIZE / (1024 * 1024), boot_info->memory_size / (1024 * 1024));
}

phys_addr_t pmm_get_free_frame() {
	if ((ssize_t) (pmm_total_usable_blocks - pmm_used_blocks) <= 0) {
		return (phys_addr_t) -1;
	}
	int index = bitmap_first_unset(pmm_bitmap, pmm_total_blocks);
	if (index == -1) {
        return (phys_addr_t) -1;
	}
	bitmap_set(pmm_bitmap, index);
	pmm_used_blocks++;
	return (phys_addr_t) (BLOCK_SIZE * index);
}

void pmm_free_frame(phys_addr_t addr) {
	int index = addr / BLOCK_SIZE;
	bitmap_unset(pmm_bitmap, index);
	pmm_used_blocks--;
}