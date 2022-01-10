#include <arch/align.h>
#include <arch/bootmem/bootmem.h>
#include <arch/bootconsole/bootconsole.h>
#include <arch/types.h>
#include <kernel/assert.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <lib/string/string.h>
#include <platform/multiboot2/multiboot2.h>

/*
 * This is the kernel architecture specific entry point after the early boot code. This reads in the architecture specific boot information and initializes the early boot console and
 * passes this information to the higher level kernel entry point.
 */

extern void kmain(bootinfo_t*);

void amain(uint32_t magic, multiboot2_information_header_t *m_boot2_info) {
	bootconsole_init(BOOTCONSOLE_VGA);
	bootinfo_t *boot_info = (bootinfo_t*) bmalloc(sizeof(bootinfo_t));
	if (!boot_info) {
		printk("Failed to allocate memory for boot_info!\n");
		return;
	}
	multiboot2_tag_header_t *tag;
	for (tag = (multiboot2_tag_header_t*) ((uintptr_t) (m_boot2_info) + 8); tag->type != MULTIBOOT2_TAG_END_TYPE;) {
		if (tag->type == MULTIBOOT2_TAG_BOOT_COMMANDLINE_TYPE) {
			multiboot2_tag_boot_command_line_t *cmd_line = (multiboot2_tag_boot_command_line_t*) tag;
			size_t n_args = 0;
			for (size_t i = 0; i < cmd_line->size - 8; i++) {
				if (cmd_line->string[i] == ' ' || cmd_line->string[i] == '\0') {
					n_args++;
				}
			}
			boot_info->karg_entries = n_args;
			karg_t *kargs = (karg_t*) bmalloc(sizeof(karg_t) * n_args);
			bool is_key = 1;
			size_t key_size = 0;
			size_t value_size = 0;
			for (size_t i = 0, j = 0; i < cmd_line->size - 8; i++) {
				if (is_key) {
					key_size++;
				}
				else {
					value_size++;
				}
				if (cmd_line->string[i] == '=') {
					char *key = (char*) bmalloc(sizeof(char) * key_size);
					if (!key) {
						printk("Failed to allocate memory for key!\n");
						return;
					}
					memcpy(key, &cmd_line->string[i - (key_size -1) ], key_size - 1);
					key[key_size] = '\0';
					kargs[j].key = key;
					key_size = 0;
					is_key = 0;
				}
				if (cmd_line->string[i] == ' ' || cmd_line->string[i] == '\0') {
					char *value = (char*) bmalloc(sizeof(char) * value_size);
					if (!value) {
						printk("Failed to allocate memory for value!\n");
						return;
					}
					memcpy(value, &cmd_line->string[i - (value_size - 1)], value_size - 1);
					value[value_size] = '\0';
					kargs[j].value = value;
					j++;
					value_size = 0;
					is_key = 1;
					if (cmd_line->string[i] == '\0') {
						break;
					}
				}
			}
			boot_info->karg_entry = kargs;
			break;
		}
		tag = ALIGN((multiboot2_tag_header_t*) ((uintptr_t) (tag) + tag->size), 8);
	}
	for (size_t i = 0; i < boot_info->karg_entries; i++) {
		if (strcmp(boot_info->karg_entry[i].key, "bootconsole") == 0) {
			if (strcmp(boot_info->karg_entry[i].value, "serial") == 0) {
				//TODO: implement bootconsole serial
				bootconsole_init(BOOTCONSOLE_SERIAL);
			}
		}
	}
	if (magic != MULTIBOOT2_MAGIC) {
		printk("This version of the kernel for the i386 architecture needs to be loaded by a Multiboot2 compliant bootloader!\n");
		return;
	}
	memory_entry_t *memory;
	for (tag = (multiboot2_tag_header_t*) ((uintptr_t) (m_boot2_info) + 8); tag->type != MULTIBOOT2_TAG_END_TYPE;) {
		if (tag->type == MULTIBOOT2_TAG_MEMORY_MAP_TYPE) {
			size_t number_available_entries = 0;
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
			entry = (multiboot2_tag_memory_map_entry_t*) memory_map->entries;
			for (size_t i = 0; i < number_entries; i++) {
				if(entry->type == MULTIBOOT2_MEMORY_AVAILABLE || entry->type == MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE){
					memory->base_addr = entry->base_addr;
					memory->length = entry->length;
					memory++;
				}
				entry++;
			}
			boot_info->memory_map_entries = number_available_entries;
			boot_info->memory_map_entry = memory - number_available_entries;
			break;
		}
		tag = ALIGN((multiboot2_tag_header_t*) ((uintptr_t) (tag) + tag->size), 8);
	}
	kmain(boot_info);
}
