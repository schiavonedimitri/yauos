#include <stdint.h>
#include <arch/arch.h>
#include <arch/cpu/io.h>
#include <kernel/bootinfo.h>
#include <kernel/pmm.h>
#include <kernel/printk.h>

void kmain(bootinfo_t *boot_info) {
	pmm_init(boot_info);
	printk("Init done\n");
}
