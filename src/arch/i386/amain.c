#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/types.h>
#include <kernel/bootconsole.h>
#include <kernel/assert.h>
#include <kernel/bootinfo.h>
#include <kernel/bootmem.h>
#include <kernel/printk.h>
#include <lib/string.h>
#include <platform/multiboot2.h>

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
	boot_info->address_space_size = 0xFFFFFFFF;
	boot_info->karg_entries = 0;
	boot_info->karg_entry = NULL;
	if (!boot_info) {
		panic("[KERNEL]: Failed to allocate memory for boot_info!\n");
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
						panic("[KERNEL]: Failed to allocate memory for key!\n");
					}
					memcpy(key, &cmd_line->string[i - (key_size -1) ], key_size - 1);
					key[key_size] = '\0';
					for (size_t i = 0; i < n_args; i++) {
						if (kargs[i].key != NULL) {
							if (strcmp(kargs[i].key, key) == 0) {
								panic("[KERNEL]: Command line has multiple definitions for the same key!\n");
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
						panic("[KERNEL]: Failed to allocate memory for value!\n");
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
						panic("[KERNEL]: Failed to allocate memory for buf.\n");
					}
					bootconsole_mem_flush_buffer(buf);
					if(!bootconsole_init(BOOTCONSOLE_SERIAL)) {
						bfree(buf);
						//serial bootconsole failed, switch back to memory ringbuffer to not waste output during switch to vga implementation.
						bootconsole_init(BOOTCONSOLE_MEM);
						printk("[KERNEL]: Failed to initialize serial bootconsole, falling back to vga text mode.\n[KERNEL]: initializing vga text mode boot console.\n");
						buf = (char*) bmalloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
						if (!buf) {
							panic("[KERNEL]: Failed to allocate memory for buf.\n");
						}
						bootconsole_mem_flush_buffer(buf);
						bootconsole_init(BOOTCONSOLE_VGA_TEXT_MODE);
						bootconsole_put_string(buf, bootconsole_mem_get_number_buffered_items());
						printk("[KERNEL]: Initialized vga text mode boot console.\n");
					}
					else {
						bootconsole_put_string(buf, bootconsole_mem_get_number_buffered_items());
						printk("[KERNEL]: Initialized serial boot console.\n");
						break;
					}
				}
				else if (boot_info->karg_entry[i].value != NULL && strcmp(boot_info->karg_entry[i].value, "vga") == 0) {
					printk("[KERNEL]: Switching boot console.\n[KERNEL]: Initializing vga text mode boot console.\n");
					char *buf = (char*) bmalloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
					if (!buf) {
						panic("[KERNEL]: Failed to allocated memory for buf!\n");
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
						panic("[KERNEL]: Failed to allocated memory for buf!\n");
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
					panic("[KERNEL]: Failed to allocated memory for buf.\n");
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
			panic("[KERNEL]: Failed to allocated memory for buf.\n");
		}
		bootconsole_mem_flush_buffer(buf);
		bootconsole_init(BOOTCONSOLE_SERIAL);
		bootconsole_put_string(buf, bootconsole_mem_get_number_buffered_items());
		printk("[KERNEL]: Initialized serial boot console.\n");
	}
	if (magic != MULTIBOOT2_MAGIC) {
		panic("[KERNEL]: This version of the kernel for the i386 architecture needs to be loaded by a Multiboot2 compliant bootloader!\n");
	}
	/*
	 * Computing bootinfo memory map entries. First we need to understand that the memory map provided by multiboot2 does not take into account
	 * all the reserved memory regions in i386 low memory (below 1Mib mark). Next the provided memory map specifies only usable regions, but we compute
	 * the memory map to take into account regions usable for mapping hardware (that is, regions that are not ram, rom or reserved) and to reserve
	 * known memory regions below 1Mib mark and the ISA memory hole at 14Mib mark. Since this is low level code specific to i386 it is safe to make this 
	 * assumptions and pass all the known reserved areas to the upper kernel as reserved regions.
	 * The memory below 1Mib mark is reserved despite multiboot2 telling it's free to be able to use vm86 mode and in case there's need to return to real mode (unlikely apart vm86 mode).
	 * See https://wiki.osdev.org/Memory_Map_(x86) for more info.
	 */
	memory_entry_t *memory;
	for (tag = (multiboot2_tag_header_t*) ((uintptr_t) (m_boot2_info) + 8); tag->type != MULTIBOOT2_TAG_END_TYPE;) {
		if (tag->type == MULTIBOOT2_TAG_MEMORY_MAP_TYPE) {
			size_t number_additional_entries = 0;
			multiboot2_tag_memory_map_t *memory_map = (multiboot2_tag_memory_map_t*) tag;
			multiboot2_tag_memory_map_entry_t *entry = (multiboot2_tag_memory_map_entry_t*) memory_map->entries;
			size_t number_entries = (memory_map->size - 16) / memory_map->entry_size;
			// Stretching the entries from the memory map to compute unspecified regions.
			for (size_t i = 0; i < number_entries; i++) {
				if((i + 1) < number_entries && entry[i].base_addr + entry[i].length != entry[i + 1].base_addr) {
					number_additional_entries++;
				}
			}
			// Allocating enough space for the original memory entries plus the additional ones made from stretching the original memory map.
			boot_info->memory_map_entries = number_entries + number_additional_entries;
			memory = (memory_entry_t*) bmalloc(sizeof(memory_entry_t) * (number_entries + number_additional_entries));
			if (!memory) {
				panic("Failed to allocate memory for memory!\n");
			}
			// Making new memory map with additonal entries made from stretching the original memory map. Old entries are copied as is in their correct location.
			for (size_t i = 0, j = 0; i < number_entries; i++, j++) {
				if (entry[i].type == MULTIBOOT2_MEMORY_AVAILABLE) {
					boot_info->memory_size += entry[i].length;
					memory[j].type = MEMORY_AVAILABLE;
				}
				else if (entry[i].type == MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE) {
					memory[j].type = MEMORY_RESERVED;
				}
				else {
					memory[j].type = MEMORY_RESERVED;
				}	
				memory[j].base_addr = entry[i].base_addr;
				memory[j].length = entry[i].length;
				if((i + 1) < number_entries && entry[i].base_addr + entry[i].length != entry[i + 1].base_addr) {
					memory[j + 1].base_addr = entry[i].base_addr + entry[i].length;
					memory[j + 1].length = entry[i + 1].base_addr - (entry[i].base_addr + entry[i].length);
					memory[j + 1].type = MEMORY_UNSPEC;
					j++;
				}				
			}
			boot_info->memory_map_entry = memory;
			break;
		}
		tag = ALIGN((multiboot2_tag_header_t*) ((uintptr_t) (tag) + tag->size), 8);
	}
	/*
	 * Calculating how many entries are there in the stretched memory map before 0x100000. This is to ensure that known < 1Mib regions of memory
	 * are correctly reserved according to the known x86 memory map. The rationale behind this is that the bios doesn't provided a memory
	 * map with real mode IVT and other regions marked as reserved. This is done to preserve the possibility to go back to real mode or vm86 mode later.
	 * All the entries (assuming they exists in the provided orignal memory map) below 0x100000 are deleted and manually crafted below.
	 */
	size_t entries_to_remove = 0;
	for (size_t i = 0; i < boot_info->memory_map_entries; i++) {
		uint64_t end_addr = boot_info->memory_map_entry[i].base_addr + boot_info->memory_map_entry[i].length - 1;
		if(end_addr <= 0xFFFFF || boot_info->memory_map_entry[i].base_addr == 0x100000) {
			entries_to_remove++;
		}
	}
	/*
	 * Computing low memory (below 1Mib) map according to https://wiki.osdev.org/Memory_Map_(x86)
	 * The new number of entries is calculated by removing all the entries below 0x100000 and the entry at 0x100000.
	 * After this there are 10 new memory entries below 1Mib mark to manually add, and 3 entries for the region above 1Mib to take into account 
	 * the ISA memory hole and the region before and after it.
	 * It would've been simpler to just set all the memory below 1Mib to reserved...
	 */
	size_t final_number_entries = boot_info->memory_map_entries - entries_to_remove + 8;
	boot_info->memory_map_entries = final_number_entries;
	memory_entry_t *final_memory_entries = (memory_entry_t*) bmalloc(sizeof(memory_entry_t) * final_number_entries);
	if (!final_memory_entries) {
		panic("Failed to allocate memory for final_memory_entries!\n");
	}
	final_memory_entries[0].base_addr = 0x0;
	final_memory_entries[0].length = 0x500;
	final_memory_entries[0].type = MEMORY_RESERVED;
	final_memory_entries[1].base_addr = 0x500;
	final_memory_entries[1].length = 0x7700;
	final_memory_entries[1].type = MEMORY_AVAILABLE;
	final_memory_entries[2].base_addr = 0x7C00;
	final_memory_entries[2].length = 0x200;
	final_memory_entries[2].type = MEMORY_RESERVED;
	final_memory_entries[3].base_addr = 0x7E00;
	final_memory_entries[3].length = 0x78200;
	final_memory_entries[3].type = MEMORY_AVAILABLE;
	final_memory_entries[4].base_addr = 0x80000;
	final_memory_entries[4].length = 0x80000;
	final_memory_entries[4].type = MEMORY_RESERVED;
	final_memory_entries[5].base_addr = 0x100000;
	final_memory_entries[5].length = 0xE00000;
	final_memory_entries[5].type = MEMORY_AVAILABLE;
	final_memory_entries[6].base_addr =  0xF00000;
	final_memory_entries[6].length = 0x100000;
	final_memory_entries[6].type = MEMORY_RESERVED;
	final_memory_entries[7].base_addr = 0x1000000;
	final_memory_entries[7].length = boot_info->memory_map_entry[entries_to_remove - 1].length - 0xF00000;
	final_memory_entries[7].type = MEMORY_AVAILABLE;
		for (size_t i = entries_to_remove, j = 8; j < final_number_entries && i < boot_info->memory_map_entries; i++, j++) {
		final_memory_entries[j].base_addr = boot_info->memory_map_entry[i].base_addr;
		final_memory_entries[j].length = boot_info->memory_map_entry[i].length;
		final_memory_entries[j].type = boot_info->memory_map_entry[i].type;
	}
	// Free old strechted memory map entries.
	bfree(boot_info->memory_map_entry);
	boot_info->memory_map_entry = final_memory_entries;
	kmain(boot_info);
}
