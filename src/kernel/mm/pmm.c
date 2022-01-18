#include <stddef.h>
#include <stdint.h>
#include <arch/arch.h>
#include <arch/align.h>
#include <arch/mmu.h>
#include <arch/types.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <lib/bitmap/bitmap.h>
#include <lib/string/string.h>
#include "pmm.h"

static phys_addr_t k_start = VIRTUAL_TO_PHYSICAL(&_KERNEL_START_);
static phys_addr_t k_end = VIRTUAL_TO_PHYSICAL(&_KERNEL_END_);
static unsigned long *pmm_bitmap = (unsigned long*) -1;
static size_t pmm_size = 0;
static size_t pmm_total_blocks = 0;
static size_t pmm_used_blocks = 0;
static size_t pmm_reserved_blocks = 0;

static void reserve_region(phys_addr_t start_addr, size_t size) {
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
		bitmap_set(pmm_bitmap, bit++);
		pmm_used_blocks++;
		pmm_reserved_blocks++;
	}
}

void pmm_init(bootinfo_t *boot_info) {
	pmm_total_blocks = boot_info->memory_size / BLOCK_SIZE;
	pmm_size = pmm_total_blocks / 8;
	void *ptr = (void*) -1;
	if (pmm_total_blocks % 8) {
		pmm_size++;
	}
	//TODO: currently the physical memory manager doesn't take into account the memory map holes and this leads to allocate memory locations which might not exist or be reserved!
	for (size_t i = 0; i < boot_info->memory_map_entries; i++) {
		ptr = (void*) -1;
		if(boot_info->memory_map_entry[i].length >= pmm_size) {
			if (boot_info->memory_map_entry[i].base_addr != (phys_addr_t) k_start) {
				if ((phys_addr_t) (boot_info->memory_map_entry[i].base_addr + pmm_size - 1) < k_start) {
					ptr = (void*) (phys_addr_t) boot_info->memory_map_entry[i].base_addr;
					break;
				}
				else {
					if (((phys_addr_t) k_end + pmm_size - 1) <= (phys_addr_t) (boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1)) {
						ptr = (void*) k_end;
						break;
					}
				}
			}
			else {
				if (((phys_addr_t) k_end + pmm_size - 1) <= (phys_addr_t) (boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1)) {
					ptr = (void*) k_end;
					break;
				}
			}
		}
	}
	if (ptr == (void*) -1) {
		printk("[KERNEL]: Failed to allocate memory for pmm bitmap!\n");
		arch_halt();
	}
	pmm_bitmap = (uintptr_t*) PHYSICAL_TO_VIRTUAL(ptr);
	memset(pmm_bitmap, 0x0, pmm_size);
	reserve_region(k_start, k_end - k_start);
	reserve_region(VIRTUAL_TO_PHYSICAL(pmm_bitmap), pmm_size);
	printk("[KERNEL]: Initialized physical memory\n[KERNEL]: Block size: %d bytes\n[KERNEL]: Total blocks: %d\n[KERNEL]: Free blocks: %d\n[KERNEL]: Reserved blocks: %d\n[KERNEL]: Used blocks: %d\n[KERNEL]: Total available memory: %d bytes\n[KERNEL]: Total memory: %d bytes\n", BLOCK_SIZE, pmm_total_blocks, pmm_total_blocks - pmm_used_blocks, pmm_reserved_blocks, pmm_used_blocks, (pmm_total_blocks - pmm_used_blocks) * BLOCK_SIZE, boot_info->memory_size);
}

phys_addr_t pmm_get_free_frame() {
	if (pmm_total_blocks - pmm_used_blocks <= 0) {
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
