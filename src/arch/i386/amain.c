#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/bootmem/bootmem.h>
#include <arch/bootconsole/bootconsole.h>
#include <arch/types.h>
#include <kernel/assert.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <lib/string/string.h>
#include <platform/multiboot2/multiboot2.h>

extern void gdt_init();
extern size_t bootconsole_mem_get_number_buffered_items();
extern void bootconsole_mem_flush_buffer(char*);
extern void kmain(bootinfo_t*);

/*
 * This is the kernel architecture specific entry point after the early boot code.
 * Here the first steps of initialization are done: architecture specific initialization (GDT, IDT, etc...), reading the memory map and formatting it in the kernel expected way,
 * initialization of the early boot console, etc...
 */

void amain(uint32_t magic, multiboot2_information_header_t *m_boot2_info) {
	gdt_init();
	bootconsole_init(BOOTCONSOLE_MEM);
	printk("[KERNEL]: Initialized memory buffered boot console.\n");
	bootinfo_t *boot_info = (bootinfo_t*) bmalloc(sizeof(bootinfo_t));
	boot_info->karg_entries = 0;
	boot_info->karg_entry = NULL;
	if (!boot_info) {
		printk("[KERNEL]: Failed to allocate memory for boot_info!\n");
		return;
	}
	//TODO: split this mess into functions and clean up the code.
	multiboot2_tag_header_t *tag;
	for (tag = (multiboot2_tag_header_t*) ((uintptr_t) (m_boot2_info) + 8); tag->type != MULTIBOOT2_TAG_END_TYPE;) {
		//TODO: split into functions and better handle corner cases for the kernel command line errors in the future.
		if (tag->type == MULTIBOOT2_TAG_BOOT_COMMANDLINE_TYPE) {
			multiboot2_tag_boot_command_line_t *cmd_line = (multiboot2_tag_boot_command_line_t*) tag;
			size_t n_args = 0;
			if (cmd_line->size == 9) { //9 is the size in bytes of an empty command line. See multiboot2.h file.
				break;
			}
			for (size_t i = 0; i < cmd_line->size - 8; i++) {
				if (cmd_line->string[i] == ' ' || cmd_line->string[i] == '\0') {
					n_args++;
				}
			}
			boot_info->karg_entries = n_args;
			karg_t *kargs = (karg_t*) bmalloc(sizeof(karg_t) * n_args);
			for (size_t i = 0; i < n_args; i++) {
				kargs[i].key = NULL;
				kargs[i].value = NULL;
			}
			bool o_break = 0;
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
				if (is_key && (cmd_line->string[i] == '=')) {
					char *key = (char*) bmalloc(sizeof(char) * key_size);
					if (!key) {
						printk("[KERNEL]: Failed to allocate memory for key!\n");
						return;
					}
					memcpy(key, &cmd_line->string[i - (key_size -1) ], key_size - 1);
					key[key_size] = '\0';
					for (size_t i = 0; i < n_args; i++) {
						if (kargs[i].key != NULL) {
							if (strcmp(kargs[i].key, key) == 0) {
								printk("[KERNEL]: Command line has multiple definitions for the same key!\n");
								o_break = 1;
								break;
							}
						}
					}
					if (o_break) {
						break;
					}
					kargs[j].key = key;
					key_size = 0;
					is_key = 0;
				}
				if (!is_key && (cmd_line->string[i] == ' ' || cmd_line->string[i] == '\0')) {
					char *value = (char*) bmalloc(sizeof(char) * value_size);
					if (!value) {
						printk("[KERNEL]: Failed to allocate memory for value!\n");
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
	if (boot_info->karg_entries != 0) {
		for (size_t i = 0; i < boot_info->karg_entries; i++) {
			if (boot_info->karg_entry[i].key != NULL && strcmp(boot_info->karg_entry[i].key, "bootconsole") == 0) {
				if (boot_info->karg_entry[i].value != NULL && strcmp(boot_info->karg_entry[i].value, "serial") == 0) {
					printk("[KERNEL]: Switching boot console.\n[KERNEL]: Initializing serial boot console.\n");
					char *buf = (char*) bmalloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
					if (!buf) {
						printk("[KERNEL]: Failed to allocated memory for buf.\n");
						return;
					}
					bootconsole_mem_flush_buffer(buf);
					bootconsole_init(BOOTCONSOLE_SERIAL);
					bootconsole_put_string(buf, bootconsole_mem_get_number_buffered_items());
					printk("[KERNEL]: Initialized serial boot console.\n");
					break;
				}
				else if (boot_info->karg_entry[i].value != NULL && strcmp(boot_info->karg_entry[i].value, "vga") == 0) {
					printk("[KERNEL]: Switching boot console.\n[KERNEL]: Initializing vga text mode boot console.\n");
					char *buf = (char*) bmalloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
					if (!buf) {
						printk("[KERNEL]: Failed to allocated memory for buf!\n");
						return;
					}
					bootconsole_mem_flush_buffer(buf);
					bootconsole_init(BOOTCONSOLE_VGA_TEXT_MODE);
					bootconsole_put_string(buf, bootconsole_mem_get_number_buffered_items());
					printk("[KERNEL]: Initialized vga text mode boot console.\n");
					break;
				}
				else {
					printk("[KERNEL]: No boot console specified in command line, defaulting to serial boot console.\n[KERNEL]: Initializing serial boot console.\n");
					char *buf = (char*) bmalloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
					if (!buf) {
						printk("[KERNEL]: Failed to allocated memory for buf!\n");
						return;
					}
					bootconsole_mem_flush_buffer(buf);
					bootconsole_init(BOOTCONSOLE_SERIAL);
					bootconsole_put_string(buf, bootconsole_mem_get_number_buffered_items());
					printk("[KERNEL]: Initialized serial boot console.\n");
					break;
				}
			}
			else {
				printk("[KERNEL]: No parameters specified in kernel command line, defaulting to serial boot console.\n[KERNEL]: Initializing serial boot console.\n");
				char *buf = (char*) bmalloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
				if (!buf) {
					printk("[KERNEL]: Failed to allocated memory for buf.\n");
					return;
				}
				bootconsole_mem_flush_buffer(buf);
				bootconsole_init(BOOTCONSOLE_SERIAL);
				bootconsole_put_string(buf, bootconsole_mem_get_number_buffered_items());
				printk("[KERNEL]: Initialized serial boot console.\n");
				break;
			}
		}
	}
	else {
		printk("[KERNEL]: No parameters specified in kernel command line, defaulting to serial boot console.\n[KERNEL]: Initializing serial boot console.\n");
		char *buf = (char*) bmalloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
		if (!buf) {
			printk("[KERNEL]: Failed to allocated memory for buf.\n");
			return;
		}
		bootconsole_mem_flush_buffer(buf);
		bootconsole_init(BOOTCONSOLE_SERIAL);
		bootconsole_put_string(buf, bootconsole_mem_get_number_buffered_items());
		printk("[KERNEL]: Initialized serial boot console.\n");
	}
	if (magic != MULTIBOOT2_MAGIC) {
		printk("[KERNEL]: This version of the kernel for the i386 architecture needs to be loaded by a Multiboot2 compliant bootloader!\n");
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
					boot_info->memory_size += entry->length;
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