#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/arch.h>
#include <arch/mmu.h>
#include <arch/types.h>
#include <arch/kernel/pmm.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <lib/bitmap.h>
#include <lib/string.h>

static phys_addr_t k_start = VIRTUAL_TO_PHYSICAL(&_KERNEL_START_);
static phys_addr_t k_end = VIRTUAL_TO_PHYSICAL(&_KERNEL_END_);
static unsigned long *pmm_bitmap = (unsigned long*) -1;
static size_t pmm_size = 0;
// Total blocks are the total for the entire address space.
static size_t pmm_total_blocks = 0;
// Blcoks that are marked as free in the memory map.
static size_t pmm_total_usable_blocks = 0;
// Blocks used by the kernel and by the frame allocator itself for now.
static size_t pmm_reserved_blocks = 0;
// Accounts for the reserved blocks aswell, since the kernel and the frame allocator themeselves pool into available blocks.
static size_t pmm_used_blocks = 0;


/*
 * Simple routine that prints the memory map to the kernel bootconsole.
 */
static void print_memory_map(bootinfo_t *boot_info) {
	printk("[KERNEL]: Memory map\n");
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


/*
 * This routine is used to mark a region of the physical address space as reserved or free to use.
 * 2 helping macros are defined in <arch/kernel/pmm.h> that call this routine with the appropriate parameters.
 */
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
 * The physical memory manager is based on a bitmap for the entire address space because it was simpler to implement it this way. This means
 * that the bitmap contains empty space for the unavailable regions or regions reported as reserved by the memory map. This wastes some ram
 * because the bitmap is now of fixed size that depends on the architecture. On x86 it is 128Kb.
 * The first step in the initialization of the system is finding the correct place in memory with enough space to hold the bitmap which does 
 * not overlap with the kernel itself. After that, all memory is initially marked as in use and the provided memory map is scanned for available
 * memory regions. For every region in the memory map marked as available, the corresponding bits in the bitmap are marked as free.
 * Finally, the bitmap region and the kernel regions themselves are marked as in use.
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
				// Rounding up to the next page boundary because we can only use whole pages.
				if (PAGE_ROUND_UP((phys_addr_t) (boot_info->memory_map_entry[i].base_addr + pmm_size - 1)) < k_start) {
					ptr = PAGE_ROUND_UP((void*) (phys_addr_t) boot_info->memory_map_entry[i].base_addr);
					break;
				}
				else {
					// Rounding up to the next page boundary because we can only use whole pages.
					if (PAGE_ROUND_UP(((phys_addr_t) k_end + pmm_size - 1)) <= (phys_addr_t) (boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1)) {
						ptr = PAGE_ROUND_UP((void*) k_end);
						break;
					}
				}
			}
			else {
				// Rounding up to the next page boundary because we can only use whole pages.
				if (PAGE_ROUND_UP(((phys_addr_t) k_end + pmm_size - 1)) <= (phys_addr_t) (boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1)) {
					ptr = PAGE_ROUND_UP((void*) k_end);
					break;
				}
			}
		}
	}
	if (ptr == (void*) -1) {
		panic("[KERNEL]: Failed to allocate memory for pmm bitmap!\n");
	}
	/*
	 * PHYSICAL_TO_VIRTUAL is needed because we are working with physical frames but to access them we need their virtual addresses.
	 * Early boot code should have mapped enough memory into the virtual address space for this to be accessible.
	 */
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
	printk("[KERNEL]: Initialized physical memory\n[KERNEL]: Block size: %d bytes\n[KERNEL]: Usable blocks: %d\n[KERNEL]: Free blocks: %d\n[KERNEL]: Reserved blocks: %d\n[KERNEL]: Total usable memory: %dMb\n[KERNEL]: Total available memory: %dMb\n", BLOCK_SIZE, pmm_total_usable_blocks, pmm_total_usable_blocks - pmm_reserved_blocks, pmm_reserved_blocks, (pmm_total_usable_blocks - pmm_reserved_blocks) * BLOCK_SIZE / (1024 * 1024), boot_info->memory_size / (1024 * 1024));
}

/* 
 * These routines are exported to the upper kernel layers and implement the interface at <kernel/pmm.h>
 */

phys_addr_t get_free_frame() {
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

void free_frame(phys_addr_t addr) {
	int index = addr / BLOCK_SIZE;
	bitmap_unset(pmm_bitmap, index);
	pmm_used_blocks--;
}