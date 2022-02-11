#include <stdalign.h>
#include <stdint.h>
#include <arch/align.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>
#include <kernel/mm/pm.h>

void kernel_main(bootinfo_t *boot_info) {
	if (k_malloc_init() != -1) {
		uint64_t *array = (uint64_t*) k_malloc(sizeof(uint64_t) * 1024);
		if (array) {
			printk("Allocated %d bytes starting at: %x\n", sizeof(uint64_t) * 1024, array);
		}
		else {
			printk("Failed to allocate memory!\n");
		}
	}
	else {
		panic("Failed to initialize kernel heap!\n");
	}
	arch_halt();
}