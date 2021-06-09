#include <stdint.h>
#include <stdbool.h>
#include "arch/i386/boot/multiboot2.h"
#include "arch/i386/cpu/i386.h"
#include "arch/i386/cpu/gdt.h"
#include "arch/i386/cpu/mmu.h"
#include "include/string/string.h"
#include "include/kernel/console.h"
#include "include/kernel/printk.h"

extern uintptr_t boot_page_directory;
extern uintptr_t _KERNEL_START_;
extern uintptr_t _KERNEL_END_;
extern uintptr_t _KERNEL_TEXT_START_;
extern uintptr_t _KERNEL_TEXT_END_;
extern uintptr_t _KERNEL_RODATA_START_;
extern uintptr_t _KERNEL_RODATA_END_;
extern uintptr_t _KERNEL_DATA_START_;
extern uintptr_t _KERNEL_DATA_END_;
extern uintptr_t _KERNEL_BSS_START_;
extern uintptr_t _KERNEL_BSS_END_;

extern void load_gdt(gdt_descriptor_t*);

gdt_descriptor_t gdt_descriptor;
gdt_entry_t gdt[3] = {
	SEGMENT_NULL,
	SEGMENT_KCODE(0, 0xFFFFFFFF),
	SEGMENT_KDATA(0, 0xFFFFFFFF)
};

void kernel_main(uint32_t magic, multiboot2_information_header_t *multiboot2_info) {
	console_initialize();
	if (magic != MULTIBOOT2_MAGIC) {
		printk("This kernel needs to be loaded by a Multiboot2 compliant bootloader!\n");
		return;
	}
	gdt_descriptor.table_size = (sizeof(gdt_entry_t) * 3) - 1;
	gdt_descriptor.table_address = &gdt[0];
	load_gdt(&gdt_descriptor);
	multiboot2_tag_header_t *tag;
	for (tag = (multiboot2_tag_header_t*) ((uint32_t) (multiboot2_info) + 8); tag->type != MULTIBOOT2_TAG_END_TYPE;) {
		if (tag->type == MULTIBOOT2_TAG_MEMORY_MAP_TYPE) {
			printk("PHYSICAL MEMORY MAP:\n");
			multiboot2_tag_memory_map_t *memory_map = (multiboot2_tag_memory_map_t*) tag;
			multiboot2_tag_memory_map_entry_t *entry = (multiboot2_tag_memory_map_entry_t*) memory_map->entries;
			size_t number_entries = (memory_map->size - 16) / memory_map->entry_size;
			size_t memory_size = 0;
			for (size_t i = 0; i < number_entries; i++) {
				printk("(%d) START: %lx LENGTH: %lx END:%lx TYPE: %s\n", i, entry->base_addr, entry->length, entry->base_addr + entry->length, memory_type_lookup_table[entry->type]);
				if(entry->type == MULTIBOOT2_MEMORY_AVAILABLE || entry->type == MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE){
					memory_size += entry->length;
				}
				entry++;
			}
			memory_size /= (1024 * 1024);
			printk("Total available RAM: %dMB\n", memory_size);
		}
		else if (tag->type == MULTIBOOT2_TAG_VBE_INFO_TYPE) {
			multiboot2_tag_vbe_info_t *vbe_info = (multiboot2_tag_vbe_info_t *)tag;
			printk("VBE MODE: %x\n", vbe_info->vbe_mode);
		}
		tag = ALIGN((multiboot2_tag_header_t*) ((uint32_t) (tag) + tag->size), 8);
	}
}
