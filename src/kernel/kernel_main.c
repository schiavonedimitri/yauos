#include <stdint.h>
#include <arch/cpu/cpu.h>
#include <arch/mmu.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>
#include <kernel/mm/pm.h>
#include <arch/kernel/mm/vm.h>

void kernel_main() {
	printk("[KERNEL]: Arch init complete.\n");
	k_malloc_init();
	for(;;) {
		size_t count = 0;
		uint32_t *p = (uint32_t*) k_malloc(sizeof(uint32_t));
		if (p != NULL) {
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
		halt();
	}
}