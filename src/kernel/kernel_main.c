#include <stddef.h>
#include <stdint.h>
#include <arch/cpu/cpu.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>

void kernel_main(bootinfo_t *boot_info) {
	printk("[KERNEL]: Arch init complete.\n[KERNEL]: Command line: \"%s\"\n", boot_info->command_line);
	if (k_malloc_init()) {
		panic("[KERNEL]: Failed to initialize heap! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
	for(;;) {
		size_t count = 0;
		uint32_t *p = (uint32_t*) k_malloc(sizeof(uint32_t));
		if (p) {
			*p = 0xDEADBEEF;
			if (*p != 0xDEADBEEF) {
				count++;
			}
		}
		else {
			printk("Allocated whole heap!\nTotal failed memory readbacks: %d\n", count);
			break;
		}
	}
	while(1) {
		arch_halt();
	}
}