#include <stddef.h>
#include <stdint.h>
#include <arch/bootmem.h>
#include <arch/defs.h>
#include <arch/mmu.h>
#include <arch/multiboot2.h>
#include <arch/pmm.h>
#include <arch/types.h>
#include <kernel/printk.h>
#include <lib/bitmap/bitmap.h>
#include <lib/string/string.h>

#define BLOCK_SIZE PAGE_SIZE

extern uintptr_t _KERNEL_START_;
extern uintptr_t _KERNEL_END_;
extern uintptr_t _KERNEL_BSS_END_;

char *memory_map_lookup_table[] = {
	"",
	"Available",
	"Reserved",
	"Acpi reclaimable",
	"Nvs",
	"Bad ram"
};

static phys_addr_t *k_start = (phys_addr_t*) VIRTUAL_TO_PHYSICAL(&_KERNEL_START_);
static phys_addr_t *k_end = (phys_addr_t*) VIRTUAL_TO_PHYSICAL(&_KERNEL_END_);
static size_t memory_size = 0;
static memory_entry_t *m_map = (memory_entry_t*) -1;
static virt_addr_t *pmm_bitmap = (virt_addr_t*) -1;
static size_t pmm_size = 0;
static size_t m_map_n_entries = 0;
static size_t pmm_total_blocks = 0;
static size_t pmm_used_blocks = 0;
static size_t pmm_reserved_blocks = 0;

static void read_memory_map(multiboot2_information_header_t *m_boot2_info) {
	size_t number_available_entries = 0;
	memory_entry_t *memory;
	multiboot2_tag_header_t *tag;
	for (tag = (multiboot2_tag_header_t*) ((uint32_t) (m_boot2_info) + 8); tag->type != MULTIBOOT2_TAG_END_TYPE;) {
		if (tag->type == MULTIBOOT2_TAG_MEMORY_MAP_TYPE) {
			multiboot2_tag_memory_map_t *memory_map = (multiboot2_tag_memory_map_t*) tag;
			multiboot2_tag_memory_map_entry_t *entry = (multiboot2_tag_memory_map_entry_t*) memory_map->entries;
			size_t number_entries = (memory_map->size - 16) / memory_map->entry_size;
			for (size_t i = 0; i < number_entries; i++) {
				if(entry->type == MULTIBOOT2_MEMORY_AVAILABLE || entry->type == MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE){
					number_available_entries++;
				}
				entry++;
			}
			memory = (memory_entry_t*) bmalloc(sizeof(memory_entry_t) * number_available_entries);
			assert((virt_addr_t*) memory != (virt_addr_t*) NULL);
			m_map_n_entries = number_available_entries;
			m_map = memory;
			entry = (multiboot2_tag_memory_map_entry_t*) memory_map->entries;
			printk("PMM: Reading memory map\n");
			for (size_t i = 0; i < number_entries; i++) {
				printk("PMM: Region [%d]: start addr: %lx end addr: %lx length: %lx type: %s\n", i, entry->base_addr, entry->base_addr + entry->length - 1, entry->length, memory_map_lookup_table[entry->type]);
				if(entry->type == MULTIBOOT2_MEMORY_AVAILABLE || entry->type == MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE){
					memory->start_addr = entry->base_addr;
					memory->end_addr = entry->base_addr + entry->length -1;
					memory->length = entry->length;
					memory_size += entry->length;
					memory++;
				}
				entry++;
			}
		}
		tag = ALIGN((multiboot2_tag_header_t*) ((uint32_t) (tag) + tag->size), 8);
	}
}

static void reserve_region(phys_addr_t* start_addr, size_t size) {
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
	uint32_t bit = (uint32_t) start_addr / BLOCK_SIZE;
	for (size_t i = 0; i < region_in_blocks; i++) {
		bitmap_set(pmm_bitmap, bit++);
		pmm_used_blocks++;
		pmm_reserved_blocks++;
	}
}

void pmm_init(multiboot2_information_header_t *m_boot2_info) {
	read_memory_map(m_boot2_info);
	pmm_total_blocks = memory_size / BLOCK_SIZE;
	pmm_size = pmm_total_blocks / 8;
	phys_addr_t *ptr = (phys_addr_t*) -1;
	if (pmm_total_blocks % 8) {
		pmm_size++;
	}
	for (size_t i = 0; i < m_map_n_entries; i++) {
		ptr = (phys_addr_t*) -1;
		if(m_map[i].length >= pmm_size) {
			if (m_map[i].start_addr != (uint32_t) k_start) {
				if ((phys_addr_t*) ((uint32_t) (ptr = (phys_addr_t*) (uint32_t) m_map[i].start_addr) + pmm_size - 1) < k_start) {
					break;
				}
				else {
					if ((phys_addr_t*) ((uint32_t) (ptr = (phys_addr_t*) k_end) + pmm_size - 1) <= (phys_addr_t*) (uint32_t) m_map[i].end_addr) {
						break;
					}
				}
			}
			else {
				if ((phys_addr_t*) ((uint32_t) (ptr = (phys_addr_t*) k_end) + pmm_size - 1) <= (phys_addr_t*) (uint32_t) m_map[i].end_addr) {
					break;
				}
			}
		}
	}
	assert(ptr != (phys_addr_t*) -1);
	pmm_bitmap = PHYSICAL_TO_VIRTUAL(ptr);
	memset(pmm_bitmap, 0x0, pmm_size);
	reserve_region(k_start, (size_t) &_KERNEL_END_ - (size_t) &_KERNEL_START_);
	reserve_region(VIRTUAL_TO_PHYSICAL(pmm_bitmap), pmm_size);
	printk("PMM: Initialized physical memory:\nBlock size: %d bytes\nTotal blocks: %d\nFree blocks: %d\nReserved blocks: %d\nUsed blocks: %d\nTotal available memory: %d bytes\nTotal memory: %d bytes\n", BLOCK_SIZE, pmm_total_blocks, pmm_total_blocks - pmm_used_blocks, pmm_reserved_blocks, pmm_used_blocks, pmm_total_blocks * BLOCK_SIZE, memory_size);
}

phys_addr_t *pmm_get_free_page() {
	if (pmm_total_blocks - pmm_used_blocks <= 0) {
		return (phys_addr_t*) -1;
	}
	int index = bitmap_first_unset(pmm_bitmap, pmm_total_blocks);
	if (index == -1) {
        return (phys_addr_t*) -1;
	}
	bitmap_set(pmm_bitmap, index);
	pmm_used_blocks++;
	return (phys_addr_t *) (BLOCK_SIZE * index);
}

void pmm_free_page(phys_addr_t *addr) {
	int index = (uint32_t) addr / BLOCK_SIZE;
	bitmap_unset(pmm_bitmap, index);
	pmm_used_blocks--;
}
