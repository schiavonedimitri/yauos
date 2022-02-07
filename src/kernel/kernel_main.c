#include <stdint.h>
#include <arch/align.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>

void kernel_main(bootinfo_t *boot_info) {
	printk("[KERNEL]: Init done.\n");
	if (k_malloc_init() != -1) {
		size_t count = 0;
		for(;;) {
			uint8_t *p = (uint8_t*) k_malloc(sizeof(uint8_t));
			if (p) {
				count++;
			}
			else {
				printk("Failed to allocate memory!\n");
				break;
			}
		}
		printk("Allocated %dMb in total.\n", sizeof(uint8_t) * count / (1024 * 1024));
	}
}