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
static size_t pmm_total_available_blocks = 0;
static size_t pmm_used_blocks = 0;
static size_t pmm_reserved_blocks = 0;

static void print_memory_map(bootinfo_t *boot_info) {
	printk("[KERNEL]: Final Memory map\n");
	for (size_t i = 0; i < boot_info->memory_map_entries; i++) {
		printk("[KERNEL]: Region: [%lx-%lx] ", boot_info->memory_map_entry[i].base_addr, boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1);
		if (boot_info->memory_map_entry[i].type == MEMORY_AVAILABLE) {
			printk("[AVAILABLE]\n");
		}
		else if (boot_info->memory_map_entry[i].type == MEMORY_RESERVED) {
			printk("[RESERVED]\n");
		}
		else {
			printk("[UNSPECIFIED]\n");
		}
	}
}

static void reserve_region(phys_addr_t start_addr, size_t size, bool account_reserved_blocks) {
	size_t region_in_blocks;
	if (size < BLOCK_SIZE) {
		region_in_blocks = 1;
	}
	else {
		region_in_blocks = size / BLOCK_SIZE;
		if (size % BLOCK_SIZE) {
			region_in_blocks++;
		}
	}
	size_t bit = start_addr / BLOCK_SIZE;
	for (size_t i = 0; i < region_in_blocks; i++) {
		/*
		 * Since the block size is a page size, it could happen that multiple regions smaller than a block are reserved in the
		 * memory map. Foresee for such cases and don't update reserved or used blocks if it was already reserved or set used.
		 */
		if (!bitmap_test(pmm_bitmap, bit)) {
			bitmap_set(pmm_bitmap, bit++);
			if (account_reserved_blocks) {
				pmm_used_blocks++;
			}
			else {
				pmm_reserved_blocks++;
			}
		}
	}
}

/*
* The kernel expects every architecture to pass it a complete memory map with every available and reserved region. See bootinfo.h
* This kernel physical memory manager is built around a bitmap and expects to build a bitmap for the entire address space of the architecture in question (that is, not only available physical ram) 
* but also any other physical area that could be used by mmio hardware or whatever else. This takes 128Kb space on a 32 bit architecture.
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
	memset(pmm_bitmap, 0x0, pmm_size);
	for (size_t i = 0; i < boot_info->memory_map_entries; i++) {
		if (boot_info->memory_map_entry[i].type == MEMORY_RESERVED || boot_info->memory_map_entry[i].type == MEMORY_UNSPEC) {
			reserve_region(boot_info->memory_map_entry[i].base_addr, boot_info->memory_map_entry[i].length, false);
		}
	}
	reserve_region(k_start, k_end - k_start, true);
	reserve_region(VIRTUAL_TO_PHYSICAL(pmm_bitmap), pmm_size, true);
	pmm_total_available_blocks = pmm_total_blocks - pmm_reserved_blocks;
	printk("[KERNEL]: Initialized physical memory\n[KERNEL]: Block size: %d bytes\n[KERNEL]: Total blocks: %d\n[KERNEL]: Free blocks: %d\n[KERNEL]: Reserved blocks: %d\n[KERNEL]: Total available memory: %d bytes\n[KERNEL]: Total memory: %d bytes\n", BLOCK_SIZE, pmm_total_available_blocks, pmm_total_available_blocks - pmm_used_blocks, pmm_used_blocks, (pmm_total_available_blocks - pmm_used_blocks) * BLOCK_SIZE, boot_info->memory_size);
}

phys_addr_t pmm_get_free_frame() {
	if ((ssize_t) (pmm_total_available_blocks - pmm_used_blocks) <= 0) {
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