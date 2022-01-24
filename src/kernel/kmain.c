#include <stdint.h>
#include <kernel/bootinfo.h>
#include <kernel/pmm.h>
#include <kernel/printk.h>

void kmain(bootinfo_t *boot_info) {
	pmm_init(boot_info);
	printk("[KERNEL]: Init done.\n");
	printk("Testing frame allocation.\n");
	for(;;) {
		phys_addr_t frame = pmm_get_free_frame();
		if (frame != (phys_addr_t) -1) {
			printk("[%x]\n", frame);
		}
		else {
			break;
		}
	}
}
