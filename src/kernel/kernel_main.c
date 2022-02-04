#include <stdint.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>


void kernel_main(bootinfo_t *boot_info) {
	printk("[KERNEL]: Init done.\n");
}
