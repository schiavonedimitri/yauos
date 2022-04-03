#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/arch.h>
#include <arch/mmu.h>
#include <arch/types.h>
#include <kernel/mm/pm.h>
#include <kernel/bootinfo.h>
#include <kernel/bootmem.h>
#include <kernel/printk.h>
#include <lib/bitmap.h>
#include <lib/string.h>

virt_addr_t kernel_virtual_end = 0;
static phys_addr_t k_start = VIRTUAL_TO_PHYSICAL(&_KERNEL_START_);
static phys_addr_t k_end = VIRTUAL_TO_PHYSICAL(&_KERNEL_END_);
static bitmap_list_t *bitmap_list = NULL;
static size_t total_blocks = 0;
static size_t total_reserved_blocks = 0;
static size_t total_used_blocks = 0;

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
 * This routine checks which bitmap manages a given address and returns it's address.
 */

static bitmap_list_t* addr_to_bitmap(phys_addr_t address) {
	for (bitmap_list_t *curr = bitmap_list;;){
		if (address <= curr->last_addr) {
			return curr;
		}
	    if (curr->next == NULL || curr->next == bitmap_list) {
			break;
		}
		curr = curr->next;
	}
	return (bitmap_list_t*) -1;
}

static int reserve_region(phys_addr_t start_addr, size_t size) {
	bitmap_list_t *bitmap = addr_to_bitmap(start_addr);
	if (bitmap == (bitmap_list_t*) -1) {
		return -1;
	}
	size_t region_in_blocks;
	if (size < BLOCK_SIZE) {
		region_in_blocks = 1;
	}
	else {
		region_in_blocks = size / BLOCK_SIZE;
	}
	size_t bit = start_addr / BLOCK_SIZE - bitmap->first_addr / BLOCK_SIZE;
	for (size_t i = 0; i < region_in_blocks; i++ , bit++) {
		if (!bitmap_test(bitmap->bitmap, bit)) {
			bitmap->reserved_blocks++;
			total_reserved_blocks++;
			bitmap->used_blocks++;
			total_used_blocks++;
			bitmap_set(bitmap->bitmap, bit);
		}
	}
	return 1;
}

/*
 * This routine initializes the physical memory manager which is based on a list of bitmaps for each available region reported by the firmware memory map.
 * This routine iterates over the firmware memory map and creates a bitmap for each region and initializes the list. Then it scans through the reported
 * available memory to find a spot large enough to hold all the bitmaps. Finally it reserves the memory used by the kernel image and the memory used
 * by each of the bitmap.
 * Note that the bitmaps themeselves are allocated from available free memory but the structures used by the manger to hold information about each bitmap
 * (which are linked in a list) are allocated from the early boot memory manager (whose memory is reserved in the kernel image itself in the bss section).
 */

void pmm_init(bootinfo_t *boot_info) {
	print_memory_map(boot_info);
	// Track down how much space we require for all the bitmaps for the available memory regions (used later for allocating space fo them).
	size_t all_bitmaps_size = 0;
	// Step 1: create a bitmap for each region of memory reported as available.
	for (size_t i = 0; i < boot_info->memory_map_entries; i++){
		if (boot_info->memory_map_entry[i].type == MEMORY_AVAILABLE || boot_info->memory_map_entry[i].type == MEMORY_RECLAIMABLE) {
			size_t bitmap_blocks = (boot_info->memory_map_entry[i].length / BLOCK_SIZE);
			//panic("curr bitmap blocks: %d\n", bitmap_blocks);
			total_blocks += bitmap_blocks;
			size_t bitmap_size = bitmap_blocks / 8;
			if (bitmap_blocks % 8) {
				bitmap_size++;
			}
			all_bitmaps_size += bitmap_size;
			bitmap_list_t *tmp = (bitmap_list_t*) b_malloc(sizeof(bitmap_list_t));
			if (!tmp) {
				panic("[KERNEL]: Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
			}
			tmp->bitmap = (uint32_t*)0xDEADBEEF;
			tmp->bitmap_size = bitmap_size;
			// Should be already rounded to a page size already...but just in case.
			tmp->first_addr = PAGE_ROUND_UP(boot_info->memory_map_entry[i].base_addr);
			tmp->last_addr = PAGE_ROUND_DOWN(boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1);
			tmp->total_blocks = bitmap_blocks;
			tmp->reserved_blocks = 0;
			tmp->used_blocks = 0;
			// If this is the first item in the list, chain it to itself.
			if (i == 0) {
				bitmap_list = tmp;
				bitmap_list->next = bitmap_list;
			}
			// Chain each element of the list to the next and to the starting one.
			else {
				bitmap_list_t *curr = bitmap_list;
				for (;;){
					if (curr->next == NULL || curr->next == bitmap_list) {
						curr->next = tmp;
						tmp->next = bitmap_list;
						break;
					}
					curr = curr->next;
				}
			}
		}
	}
	// Update kernel virtual end address to take into account the virtual space taken by the bitmaps.
	kernel_virtual_end = PHYSICAL_TO_VIRTUAL(k_end) + all_bitmaps_size;
	// Will point to an area of memory large enough to hold all the bitmaps.
	void *start_available_memory = (void*) -1;
	// Try to find a place big enough to hold all the memory bitmaps that doesn't fall in the kernel range.
	for (size_t i = 0; i < boot_info->memory_map_entries; i++) {
		start_available_memory = (void*) -1;
		if (boot_info->memory_map_entry[i].type == MEMORY_AVAILABLE || boot_info->memory_map_entry[i].type == MEMORY_RECLAIMABLE) {
			// If the architecute is i386 try to place the bitmaps above 0x100000 to avoid overwriting BDA, EBDA and other known x86 low memory regions.
			#if ARCH == i386
				if(boot_info->memory_map_entry[i].base_addr < 0x100000) {
					continue;
				}
			#endif
			// Check if this region is big enough to hold all the bitmaps.
			if (boot_info->memory_map_entry[i].length >= all_bitmaps_size) {
				// Check if the region starting address is also the kernel starting address.
				if (boot_info->memory_map_entry[i].base_addr != k_start) {
					// Check if Kernel starts in this region.
					if (k_start <= boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length) {
						// Check that there's enough space from base address for the bitmaps to not collide with kernel starting address.
						// Rounding up to the next page boundary because we can only use whole pages.
						if (PAGE_ROUND_UP((phys_addr_t) (boot_info->memory_map_entry[i].base_addr + all_bitmaps_size - 1)) < k_start) {
							start_available_memory = PAGE_ROUND_UP((void*) (phys_addr_t) boot_info->memory_map_entry[i].base_addr);
							break;
						}
						// Check if the kernel ends in this region and skip if it doesn't (means it spawns all the region's remaining space).
						else {
							if (k_end >= boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1) {
								continue; 
							}
							// The kernel doesn't spawn the entire region, check that enough space is left for the bitmaps.
							// Rounding up to the next page boundary because we can only use whole pages.
							if (PAGE_ROUND_UP(((phys_addr_t) k_end + all_bitmaps_size - 1)) <= (phys_addr_t) (boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1)) {
								start_available_memory = PAGE_ROUND_UP((void*) k_end);
								break;
							}
						}
					}
					// The kernel doesn't start in this region, check that there's enough space for the bitmaps.
					else {
						// Rounding up to the next page boundary because we can only use whole pages.
						if (PAGE_ROUND_UP((phys_addr_t) (boot_info->memory_map_entry[i].base_addr + all_bitmaps_size - 1)) <= (phys_addr_t) (boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1)) {
							start_available_memory = PAGE_ROUND_UP((void*) (phys_addr_t) boot_info->memory_map_entry[i].base_addr);
							break;
						}

					}
				}
				// Kernel starts at this region starting address.
				else {
					// Check that the kernel doesn't spawn this entire region, in case skip to the next.
					if (k_end >= boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1) {
						continue; 
					}
					// The kernel doesn't spawn the entire region, check that enough space is left for the bitmaps.
					// Rounding up to the next page boundary because we can only use whole pages.
					if (PAGE_ROUND_UP(((phys_addr_t) k_end + all_bitmaps_size - 1)) <= (phys_addr_t) (boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1)) {
						start_available_memory = PAGE_ROUND_UP((void*) k_end);
						break;
					}
				}
			}
		}
	}
	if (start_available_memory == (void*) -1) {
		panic("[KERNEL]: Could not find an area of memory for the memory manager itself! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
	// Allocate memory for the bitmaps from the region just found above.
	for (bitmap_list_t *curr = bitmap_list;;) {
		curr->bitmap = (uint32_t*) PHYSICAL_TO_VIRTUAL(start_available_memory);
		// Mark all the memory of this bitmap as free to use.
		memset(curr->bitmap, 0x0, curr->bitmap_size);
		// Mark the starting address for the next bitmap.
		start_available_memory = (void*) (size_t) start_available_memory + curr->bitmap_size;
		if (curr->next == NULL || curr->next == bitmap_list) {
			break;
		}
		curr = curr->next;
	}
	// If the architecture is i386 resreve known low memory regions.
	#if ARCH == i386
	if (reserve_region(0x0, 0x3FF) == -1) {
		panic("[KERNEL]: Could not reserve IVT memory region! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
	if (reserve_region(0x400, 0x4FF - 0x400) == -1) {
		panic("[KERNEL]: Could not reserve BDA memory region! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
	if (reserve_region(0x80000, 0x9FFFF - 0x80000) == -1) {
		panic("[KERNEL]: Could not reserve EBDA memory region! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
	#endif
	// Reserving kernel memory.
	if (reserve_region(k_start, k_end - k_start) == -1) {
		panic("[KERNEL]: Could not reserve kernel physical memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
	// Reserving memory used by each bitmap.
	for (bitmap_list_t *curr = bitmap_list;;) {
		if (reserve_region(VIRTUAL_TO_PHYSICAL(curr->bitmap), curr->bitmap_size) == -1) {
			panic("[KERNEL]: Could not reserve memory for physical memory manager bitmap! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);	
		}
		if (curr->next == NULL || curr->next == bitmap_list) {
			break;
		}
		curr = curr->next;
	}
	// Set the memory regions of reclaimable memory as reserved initially.
	for (size_t i = 0; i < boot_info->memory_map_entries; i++) {
		if (boot_info->memory_map_entry[i].type == MEMORY_RECLAIMABLE) {
			reserve_region(boot_info->memory_map_entry[i].base_addr, boot_info->memory_map_entry[i].length);
		}
	}
	printk("[KERNEL]: Initialized physical memory\n[KERNEL]: Block size: %d bytes\n[KERNEL]: Total blocks: %d\n[KERNEL]: Reserved blocks: %d\n[KERNEL]: Used blocks: %d\n[KERNEL]: Total usable memory: %dMb\n[KERNEL]: Total available memory: %dMb\n", BLOCK_SIZE, total_blocks, total_reserved_blocks, total_used_blocks, (total_blocks - total_reserved_blocks) * BLOCK_SIZE / (1024 * 1024), boot_info->memory_size / (1024 * 1024));
}

/* 
 * These routines are exported to the upper kernel layers and implement the interface at <kernel/pm.h>
 */

phys_addr_t get_free_frame() {
	// This is used to remember the last bitmap with free blocks. Initially it is the first bitmap.
	static bitmap_list_t *last_with_free_blocks = NULL;
	// Check to see if there is any free block in the system.
	if ((ssize_t) total_blocks - total_used_blocks <= 0){
		return -1;
	}
	bitmap_list_t *curr = NULL;
	if (last_with_free_blocks != NULL) {
		curr = last_with_free_blocks;
	}
	else {
		curr = bitmap_list;
	}
	for (;;) {
		// Check if this bitmap has any free blocks.
		if ((ssize_t) curr->total_blocks - curr->used_blocks > 0) {
			int index = bitmap_first_unset(curr->bitmap, curr->total_blocks);
			if (index != -1) {
        		bitmap_set(curr->bitmap, index);
				curr->used_blocks++;
				total_used_blocks++;
				// If we allocated the last block of this bitmap, set the future search to begin at the next bitmap.
				if (curr->used_blocks == curr->total_blocks) {
					last_with_free_blocks = curr->next;
				}
				return (phys_addr_t) (BLOCK_SIZE * index) + curr->first_addr;
			}
		}
		if (curr->next == NULL || curr->next == bitmap_list) {
			break;
		}
		curr = curr->next;
	}
	return -1;
}

void free_frame(phys_addr_t addr) {
	bitmap_list_t *bitmap = addr_to_bitmap(addr);
	if (bitmap == (bitmap_list_t*) -1) {
		panic("[PM]: Could not find address to free! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
	int index = addr / BLOCK_SIZE - bitmap->first_addr / BLOCK_SIZE;
	if (!bitmap_test(bitmap->bitmap, index)) {
		panic("[PM]: Trying to free a block already free! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
	bitmap_unset(bitmap->bitmap, index);
	bitmap->used_blocks--;
	total_used_blocks--;
}