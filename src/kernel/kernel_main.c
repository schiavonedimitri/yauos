#include <stdint.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>

void kmain(bootinfo_t *boot_info) {
	printk("[KERNEL]: Init done.\n");
}
