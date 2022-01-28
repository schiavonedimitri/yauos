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
bootinfo_t *boot_info;

static void parse_cmdline(multiboot2_information_header_t *m_boot2_info) {
	boot_info->karg_entries = 0;
	boot_info->karg_entry = NULL;
	multiboot2_tag_header_t *tag;
	for (tag = (multiboot2_tag_header_t*) ((uintptr_t) (m_boot2_info) + 8); tag->type != MULTIBOOT2_TAG_END_TYPE;) {
		//TODO: Better handle corner cases for the kernel command line errors.
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
								break;
							}
						}
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
}

static void parse_memory_map(multiboot2_information_header_t *m_boot2_info) {
	multiboot2_tag_header_t *tag;
	/*
	 * Copy the memory map provided by the bootloader to the kernel bootinfo structure.
	 * Note: this kernel does not support real mode applications or vm86 tasks, so memory 
	 * starting at 0x0 (if it is reported to exist) is freely used.
	 * Note 2: Since this is a hobby educational project mainly used to learn by doing
	 * not all possible scenarios have been covered and some assumptions that simplify coding have been made.
	 * One of these assumptions is that the firmware provides a sane memory map.
	 * This code has been run mainly on Qemu and it's memory map looks sane enough.
	 * This code has also been tested this on some real machines (mainly 64 bit systems with UEFI-CSM) and the memory
	 * maps there have also been confirmed to be sane enough. 
	 */
	for (tag = (multiboot2_tag_header_t*) ((uintptr_t) (m_boot2_info) + 8); tag->type != MULTIBOOT2_TAG_END_TYPE;) {
		if (tag->type == MULTIBOOT2_TAG_MEMORY_MAP_TYPE) {
			multiboot2_tag_memory_map_t *memory_map = (multiboot2_tag_memory_map_t*) tag;
			multiboot2_tag_memory_map_entry_t *entry = (multiboot2_tag_memory_map_entry_t*) memory_map->entries;
			size_t number_entries = (memory_map->size - 16) / memory_map->entry_size;
			/* 
			 * This kernel does not support PAE so we skip entries above 0xFFFFFFFF. If we add the 3Gb memory hole
			 * this means that on this architecture the maximum usable ram is limited to 3Gb because any ram pushed 
			 * above 0xFFFFFFFF is not addressable.
			 * The calculation of the index below is done to allocate only the number of entries we want.
			 */
			size_t number_entries_final = 0;
			for (size_t i = 0; i < number_entries; i++) {
				if (entry[i].base_addr > 0xFFFFFFFF) {
					break;
				}
				number_entries_final++;
			}
			boot_info->memory_map_entries = number_entries_final;
			memory_entry_t *memory = (memory_entry_t*) bmalloc(sizeof(memory_entry_t) * number_entries_final);
			if (!memory) {
				panic("Failed to allocate memory for memory!\n");
			}
			// Copy only the entries below 0xFFFFFFFF.
			for (size_t i = 0; i < number_entries; i++) {
				/* 
				 * This is because we don't know if entries in the map are in sorted order and to 
				 * display the user all available memory even if it cannot be used. (usually they are but cannot be sure about it).
				 */
				if (entry[i].base_addr > 0xFFFFFFFF) {
					boot_info->memory_size += entry[i].length;
					continue;
				}
				if (entry[i].type == MULTIBOOT2_MEMORY_AVAILABLE) {
					boot_info->memory_size += entry[i].length;
					memory[i].type = MEMORY_AVAILABLE;
				}
				else if (entry[i].type == MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE) {
					boot_info->memory_size += entry[i].length;
					memory[i].type = MEMORY_RECLAIMABLE;
				}
				else {
					memory[i].type = MEMORY_RESERVED;
				}	
				memory[i].base_addr = entry[i].base_addr;
				memory[i].length = entry[i].length;		
			}
			boot_info->memory_map_entry = memory;
			break;		
		}			
		tag = ALIGN((multiboot2_tag_header_t*) ((uintptr_t) (tag) + tag->size), 8);
	}
}

/*
 * This routine parses the kernel command line that has been made for the upper level kernel to check if 
 * any bootconsole=xxx parameter has been specified in the command line and try to initialize that bootconsole.
 * If nothing is specified the kernel fallbacks to the serial implementation and if that fails too it fallbacks to 
 * the vga implementation. The code assumes the presence of the standard vga text mode on the target platform (i386-pc).
 * Note: the makefile by default is set to use serial bootconsole. If you try to run on real hardware remember to set
 * that parameter to vga or ensure to have a serial connection working.
 */

static void boot_console_init() {
	if (boot_info->karg_entries != 0) {
		for (size_t i = 0; i < boot_info->karg_entries; i++) {
			if (boot_info->karg_entry[i].key != NULL && strcmp(boot_info->karg_entry[i].key, "bootconsole") == 0) {
				// Serial bootconsole was specified on the command line.
				if (boot_info->karg_entry[i].value != NULL && strcmp(boot_info->karg_entry[i].value, "serial") == 0) {
					printk("[KERNEL]: Switching boot console.\n[KERNEL]: Initializing serial boot console.\n");
					char *buf = (char*) bmalloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
					if (!buf) {
						panic("[KERNEL]: Failed to allocate memory for buf.\n");
					}
					bootconsole_mem_flush_buffer(buf);
					if(!bootconsole_init(BOOTCONSOLE_SERIAL)) {
						bfree(buf);
						// Serial bootconsole failed, switch back to memory ringbuffer to not waste output during switch to vga implementation.
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
				// vga bootconsole was specified on the command line.
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
				// Empty bootconsole value was specified, default to serial.
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
			// No bootconsole key was found on the command line.
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
	// Command line is empty, default to serial bootconsole.
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
}

/*
 * This is the kernel architecture specific entry point after the early boot code.
 * Here the first steps of initialization are done: architecture specific initialization (GDT, IDT, etc...), reading the memory map and formatting it in the kernel expected way,
 * initialization of the early boot console, etc...
 */

void amain(uint32_t magic, multiboot2_information_header_t *m_boot2_info) {
	extern void pmm_init(bootinfo_t*);
	// Setup GDT
	gdt_init();
	// Setup initial bootconsole device to the memory ringbuffer for early output.
	bootconsole_init(BOOTCONSOLE_MEM);
	printk("[KERNEL]: Initialized memory buffered boot console.\n");
	boot_info = (bootinfo_t*) bmalloc(sizeof(bootinfo_t));
	if (!boot_info) {
		panic("[KERNEL]: Failed to allocate memory for boot_info!\n");
	}
	boot_info->address_space_size = 0xFFFFFFFF;
	boot_info->memory_size = 0;
	// Parse the kernel command line and format it for easier access to the upper kernel layer.
	parse_cmdline(m_boot2_info);
	// Initialize the real bootconsole according to kernel command line or defaults.
	boot_console_init();
	// Parse the multiboot2 memory map and format it according to the way the upper kernel layer expects it.
	parse_memory_map(m_boot2_info);	
	pmm_init(boot_info);
	// If the kernel wasn't loaded by a multiboot2 compliant bootloader fail as we rely on the provided memory map.
	if (magic != MULTIBOOT2_MAGIC) {
		panic("[KERNEL]: This version of the kernel for the i386 architecture needs to be loaded by a Multiboot2 compliant bootloader!\n");
	}
	kmain(boot_info);
}