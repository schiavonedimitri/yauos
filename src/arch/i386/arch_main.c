#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/cpu/cpu.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/idt.h>
#include <arch/cpu/mp.h>
#include <arch/kernel/mm/vm.h>
#include <arch/paging.h>
#include <arch/types.h>
#include <kernel/bootconsole.h>
#include <kernel/assert.h>
#include <kernel/bootinfo.h>
#include <kernel/bootmem.h>
#include <kernel/interrupt.h>
#include <kernel/printk.h>
#include <kernel/mm/pm.h>
#include <lib/string.h>
#include <platform/multiboot2.h>
#include <platform/lapic.h>
#include <platform/pic.h>
#include <platform/pit.h>

extern uintptr_t _binary_boot_ap_start;
extern uintptr_t _binary_boot_ap_size;
extern size_t bootconsole_mem_get_number_buffered_items();
extern void bootconsole_mem_flush_buffer(char*);
// Defined in kernel/pm.c
extern void pmm_init(bootinfo_t*);
// Arch independent entry point.
extern void kernel_main(bootinfo_t*);

bootinfo_t *boot_info;
bool ap_started = 0;

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
				// Calculate how much to allocate for the kernel arguments. Args are separated by a space or null character.
				if (cmd_line->string[i] == ' ' || cmd_line->string[i] == '\0') {
					n_args++;
				}
			}
			boot_info->karg_entries = n_args;
			karg_t *kargs = (karg_t*) b_malloc(sizeof(karg_t) * n_args);
			if (!kargs) {
				panic("[KERNEL]: Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
			}
			// Set all entries as NULL initially.
			for (size_t i = 0; i < n_args; i++) {
				kargs[i].key = NULL;
				kargs[i].value = NULL;
			}
			/*
			 * While looping over the command line string, is_key is 1 until a key is succesfully parsed, then it is set to 0 and
			 * a value is parsed next. This goes on until a value is being parsed and the NULL character is found.
			 */
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
				// = is the ending character for a key because we support key value pairs in the format KEY=VAL.
				if (is_key && (cmd_line->string[i] == '=')) {
					char *key = (char*) b_malloc(sizeof(char) * key_size);
					if (!key) {
						panic("[KERNEL]: Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
					}
					/*
					 * key_size takes into account the = character for its size calculation, thus the key is copied
					 * until key_size - 1 to leave space for the NULL character which replaces the = character.
					 */
					memcpy(key, &cmd_line->string[i - (key_size - 1) ], key_size - 1);
					key[key_size] = '\0';
					// Check if this key already exists and panic in case it does. Keys should be unique.
					for (size_t i = 0; i < n_args; i++) {
						if (kargs[i].key != NULL) {
							if (strcmp(kargs[i].key, key) == 0) {
								panic("[KERNEL]: Command line has multiple definitions for the same key! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
								break;
							}
						}
					}
					kargs[j].key = key;
					// Reset key size for the next parse
					key_size = 0;
					// Indicates the start of a value parse.
					is_key = 0;
				}
				// The same as above but for a value. We expect a space or a NULL character after a key.
				if (!is_key && (cmd_line->string[i] == ' ' || cmd_line->string[i] == '\0')) {
					char *value = (char*) b_malloc(sizeof(char) * value_size);
					if (!value) {
						panic("[KERNEL]: Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
					}
					memcpy(value, &cmd_line->string[i - (value_size - 1)], value_size - 1);
					value[value_size] = '\0';
					kargs[j].value = value;
					j++;
					// Reset value size for the next parse.
					value_size = 0;
					// Indicates a new key parse.
					is_key = 1;
					// If a NULL character is found while parsing a value it means we are at the end of the command line.
					if (cmd_line->string[i] == '\0') {
						break;
					}
				}
			}
			// Set the parsed entries in the bootinfo structure.
			boot_info->karg_entry = kargs;
			break;
		}
		// Multiboot2 tags are 8 byte aligned. See multiboo2.h for more info.
		tag = ALIGN((multiboot2_tag_header_t*) ((uintptr_t) (tag) + tag->size), 8);
	}
}

static void parse_memory_map(multiboot2_information_header_t *m_boot2_info) {
	multiboot2_tag_header_t *tag;
	/*
	 * Copy the memory map provided by the bootloader to the kernel bootinfo structure.
	 * Note: Since this is a hobby educational project mainly used to learn by doing
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
			memory_entry_t *memory = (memory_entry_t*) b_malloc(sizeof(memory_entry_t) * number_entries_final);
			if (!memory) {
				panic("Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
			}
			// Copy only the entries below 0xFFFFFFFF.
			for (size_t i = 0; i < number_entries; i++) {
				/* 
				 * This is because we don't know if entries in the map are in sorted order and to 
				 * display the user all available memory even if it cannot be used. (usually they are but cannot be sure about it).
				 */
				if (entry[i].base_addr > 0xFFFFFFFF) {
					// This is just to track all the available memory (even if it is not usable by this kernel, for statistical purposes).
					if (entry[i].type == MULTIBOOT2_MEMORY_AVAILABLE || entry[i].type == MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE) {
						boot_info->memory_size += entry[i].length;					
					}
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
 * any bootconsole=xxx parameter has been specified in the command line and tries to initialize that bootconsole.
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
					char *buf = (char*) b_malloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
					if (!buf) {
						panic("[KERNEL]: Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
					}
					bootconsole_mem_flush_buffer(buf);
					if(!bootconsole_init(BOOTCONSOLE_SERIAL)) {
						b_free(buf);
						// Serial bootconsole failed, switch back to memory ringbuffer to not waste output during switch to vga implementation.
						bootconsole_init(BOOTCONSOLE_MEM);
						printk("[KERNEL]: Failed to initialize serial bootconsole, falling back to vga text mode.\n[KERNEL]: initializing vga text mode boot console.\n");
						buf = (char*) b_malloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
						if (!buf) {
							panic("[KERNEL]: Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
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
					char *buf = (char*) b_malloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
					if (!buf) {
						panic("[KERNEL]: Failed to allocated memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
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
					char *buf = (char*) b_malloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
					if (!buf) {
						panic("[KERNEL]: Failed to allocated memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
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
				char *buf = (char*) b_malloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
				if (!buf) {
					panic("[KERNEL]: Failed to allocated memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
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
		char *buf = (char*) b_malloc(sizeof(char) * bootconsole_mem_get_number_buffered_items());
		if (!buf) {
			panic("[KERNEL]: Failed to allocated memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
		}
		bootconsole_mem_flush_buffer(buf);
		bootconsole_init(BOOTCONSOLE_SERIAL);
		bootconsole_put_string(buf, bootconsole_mem_get_number_buffered_items());
		printk("[KERNEL]: Initialized serial boot console.\n");
	}
}

/*
 * Code from here and beyond is just a mess that i was doing for testing smp booting.
 * Now that i got it working it's time to properly setup things.
 */


/*
 * This are just a page directory for the ap cpus and 2 page tables.
 * The first page table is for mapping the first 2Mb physical RAM at virtual addresses 0xC0100000 and the identity map.
 * Just like on the bsp cpu. The second page table is because i decided to allocate the ap cpus stacks starting at virtual
 * address 0xD0000000.
 */

__attribute__((__aligned__(PAGE_SIZE))) page_directory_t ap_boot_page_directory;
__attribute__((__aligned__(PAGE_SIZE)))page_table_t ap_boot_page_table_0;
__attribute__((__aligned__(PAGE_SIZE)))page_table_t ap_boot_page_table_1;

/*
 * This is used to count milliseconds elapsed since the counter began counting.
 */
static volatile uint32_t elapsed_milliseconds = 0;


/*
 * This is used for the delays required for the smp ap startup code.
 */

void timer_callback(interrupt_context_t *context) {
	elapsed_milliseconds++;
	// For now the count value was obtained manually to get an IRQ every ~1ms.
	pit_interrupt_on_terminal_count(1200);
}

/*
 * This is used for delaying for a specified amount of milliseconds.
 */
void delay(uint32_t ms) {
	uint32_t end = elapsed_milliseconds + ms;
	while (elapsed_milliseconds < end);
}

void ap_main(uint32_t apic_id, uint32_t ms) {
	ap_started = 1;
	printk("Ap[%d] started!\n", apic_id);
	while(1) {
		delay(ms);
		printk("Ap[%d]: Hello!\n", apic_id);
	}	
}

/*
 * This is the kernel architecture specific entry point after the early boot code.
 * Here the first steps of initialization are done: architecture specific initialization (GDT, IDT, etc...), reading the memory map and formatting it in the kernel expected way,
 * initialization of the early boot console, etc...
 */

void arch_main(uint32_t magic, multiboot2_information_header_t *m_boot2_info) {
	/*
	 * Gdt and idt are the first steps of initialization to be able to catch any early kernel exceptions (bugs, hopefully none)
	 * and panic.
	 */
	// Base gdt initialization.
	gdt_init();
	// Base idt initialization.
	idt_init();
	/* 
	 * Setup initial bootconsole device to the memory ringbuffer for early output.
	 * The bootconsole is the first thing initialized after gdt and idt to have significant output in case of boot failure or in case 
	 * of in kernel bugs (exceptions). Only after the multiboot2 magic code is checked the rest of the initialization can proceed.
	 */
	bootconsole_init(BOOTCONSOLE_MEM);
	boot_info = (bootinfo_t*) b_malloc(sizeof(bootinfo_t));
	if (!boot_info) {
		panic("[KERNEL]: Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
	boot_info->memory_size = 0;
	// TODO: move the command line parsing one layer above the architecture specific code and initialize the early console with a fixed default one.
	// Parse the kernel command line and format it for easier access to the upper kernel layer.
	parse_cmdline(m_boot2_info);
	// Initialize the real bootconsole according to kernel command line or defaults.
	boot_console_init();
	// If the kernel wasn't loaded by a multiboot2 compliant bootloader fail as we rely on the provided memory map.
	if (magic != MULTIBOOT2_MAGIC) {
		panic("[KERNEL]: This kernel must be loaded by a Multiboot2 compliant bootloader! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
	// Parse the multiboot2 memory map and format it according to the way the upper kernel layer expects it.
	parse_memory_map(m_boot2_info);	
	// Pic initialization.
	pic_init();
	mp_init();
	pmm_init(boot_info);
	/* This messy shit was just for testing smp booting...now that it works it's time to organize things properly.
	 * First i need to implement a virtual memory address space allocator for in kernel use because i cannot
	 * randomly map physical addresses and remember them mentally.
	 */
	if (smp) {
		// Identity mapping local apic address to the same virtual address.
        map_page(local_apic_address, local_apic_address, PROT_PRESENT | PROT_READ_WRITE | PROT_KERN | PROT_CACHE_DISABLE);
		printk("System is MP compliant!\nFound %d cpus!\n", num_cpus);
		for (size_t i = 0; i < num_cpus; i++) {
			printk("CPU [%s] with ID: %x\n", cpu_data[i].bsp ? "BSP" : "AP", cpu_data[i].lapic_id);
		}
		printk("local apic physical address (identity mapped) on each cpu is: %x\nStarting application processors...\n", local_apic_address);
		lapic_init();
		register_interrupt_handler(32, timer_callback);
		pic_enable_irq_line(0);
		sti();
		pit_interrupt_on_terminal_count(1200);
		void *dest = (void*) PHYSICAL_TO_VIRTUAL(0x1000);
		memcpy(dest, &_binary_boot_ap_start, (size_t) &_binary_boot_ap_size);
		// Map first 2Mb physical RAM.
		for (size_t i = 0; i < 512; i++) {
			ap_boot_page_table_0.entry[i].present = 1;
			ap_boot_page_table_0.entry[i].address = (i*4096) >> 12;
			ap_boot_page_table_0.entry[i].read_write = 1;
			ap_boot_page_table_0.entry[i].user_supervisor = 0;
		}
		// Identity map 2Mb physical to 2Mb virtual.
		ap_boot_page_directory.entry[0].present = 1;
		ap_boot_page_directory.entry[0].address = (uint32_t) VIRTUAL_TO_PHYSICAL(&ap_boot_page_table_0) >> 12;
		ap_boot_page_directory.entry[0].read_write = 1;
		ap_boot_page_directory.entry[0].user_supervisor = 0;
		// Map 2Mb physical to 0xC0100000 virtual.
		ap_boot_page_directory.entry[768].present = 1;
		ap_boot_page_directory.entry[768].address = (uint32_t) VIRTUAL_TO_PHYSICAL(&ap_boot_page_table_0) >> 12;
		ap_boot_page_directory.entry[768].read_write = 1;
		ap_boot_page_directory.entry[768].user_supervisor = 0;
		// Map whatever physical frame we get for ap cpus stack to virtual addresses starting at 0xD0000000.
		ap_boot_page_directory.entry[832].present = 1;
		ap_boot_page_directory.entry[832].address = (uint32_t) VIRTUAL_TO_PHYSICAL(&ap_boot_page_table_1) >> 12;
		ap_boot_page_directory.entry[832].read_write = 1;
		ap_boot_page_directory.entry[832].user_supervisor = 0;
		virt_addr_t ap_stack_virtual = 0xD0001000;
		for (size_t i = 1; i < num_cpus; i++) {
			void *ap_code = (void*) PHYSICAL_TO_VIRTUAL(0x1000);
			phys_addr_t ap_stack = get_free_frame();
			if (ap_stack == (phys_addr_t) -1) {
				panic("[KERNEL]: Failed to allocated memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
			}
			// Map ap cpus stack vo virtual address starting at 0xD0000000 in the kernel directory.
			map_page(ap_stack, ap_stack_virtual, PROT_PRESENT | PROT_READ_WRITE | PROT_KERN);
			// Map ap cpus stack to virtual address starting at 0xD0000000 in the ap boot page directory.
			ap_boot_page_table_1.entry[i].present = 1;
			ap_boot_page_table_1.entry[i].address = ap_stack >> 12;
			ap_boot_page_table_1.entry[i].read_write = 1;
			ap_boot_page_table_1.entry[i].user_supervisor = 0;
			*(void**) (ap_code - 4) = (uint8_t*) ap_stack_virtual + 4096;
			*(void**) (ap_code - 8) = VIRTUAL_TO_PHYSICAL(&ap_boot_page_directory);
			*(void**) (ap_code - 12) = (void*) ap_main;
			*(void**) (ap_code - 16) = (void*) i;
			*(void**) (ap_code - 20) = (void*) (i * 200 + 100);
			lapic_send_ipi(cpu_data[i].lapic_id, LAPIC_ICR_DELIVERY_MODE_INIT | LAPIC_ICR_DESTINATION_MODE_PHYSICAL | LAPIC_ICR_LEVEL_ASSERT | LAPIC_ICR_TRIGGER_MODE_LEVEL | LAPIC_ICR_DESTINATION_NO_SHORTHAND);
			do {
				asm("pause");
			} while (LAPIC_ICR_DELIVERY_STATUS(lapic_read(LAPIC_INTERRUPT_COMMAND_REGISTER_0)) != LAPIC_ICR_DELIVERY_STATUS_IDLE);
			lapic_send_ipi(cpu_data[i].lapic_id, LAPIC_ICR_DELIVERY_MODE_INIT | LAPIC_ICR_DESTINATION_MODE_PHYSICAL | LAPIC_ICR_LEVEL_DE_ASSERT | LAPIC_ICR_TRIGGER_MODE_LEVEL | LAPIC_ICR_DESTINATION_NO_SHORTHAND);
			do {
				asm("pause");
			} while (LAPIC_ICR_DELIVERY_STATUS(lapic_read(LAPIC_INTERRUPT_COMMAND_REGISTER_0)) != LAPIC_ICR_DELIVERY_STATUS_IDLE);
			delay(10);
			// For now just send 1 SIPI and later add logic to send again and delay more increasingly.
			lapic_send_ipi(cpu_data[i].lapic_id, ((uint32_t) VIRTUAL_TO_PHYSICAL(ap_code) >> 12) | LAPIC_ICR_DELIVERY_MODE_STARTUP | LAPIC_ICR_DESTINATION_MODE_PHYSICAL | LAPIC_ICR_TRIGGER_MODE_LEVEL| LAPIC_ICR_DESTINATION_NO_SHORTHAND);
			// Delaying more than the 200 micro seconds specified in the spec.
			delay(1);
			do {
				asm("pause");
			}
			while (LAPIC_ICR_DELIVERY_STATUS(lapic_read(LAPIC_INTERRUPT_COMMAND_REGISTER_0)) != LAPIC_ICR_DELIVERY_STATUS_IDLE);
			ap_stack_virtual += 4096;
		}
	}
	kernel_main(boot_info);
}