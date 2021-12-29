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

extern uintptr_t _KERNEL_START_;
extern uintptr_t _KERNEL_END_;

static phys_addr_t *k_start = (phys_addr_t*) VIRTUAL_TO_PHYSICAL(&_KERNEL_START_);
static phys_addr_t *k_end = (phys_addr_t*) VIRTUAL_TO_PHYSICAL(&_KERNEL_END_);
static size_t memory_size = 0;
static size_t m_map_size = 0;
static phys_addr_t *pmm_bitmap = (phys_addr_t*) -1;
static memory_entry_t *m_map = (memory_entry_t*) -1;

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
			assert(memory);
			m_map_size = number_available_entries;
			assert((virt_addr_t*) memory != (virt_addr_t*) 0);
			m_map = memory;
			entry = (multiboot2_tag_memory_map_entry_t*) memory_map->entries;
			for (size_t i = 0; i < number_entries; i++) {
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

void pmm_init(multiboot2_information_header_t *m_boot2_info) {
	read_memory_map(m_boot2_info);
	size_t max_blocks = memory_size / PAGE_SIZE;
	size_t pmm_size = max_blocks / 8;
	phys_addr_t *ptr = (phys_addr_t*) -1;
	if (max_blocks % 8) {
		pmm_size++;
	}
	for (size_t i = 0; i < m_map_size; i++) {
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
	pmm_bitmap = ptr;
}
