#include <stdint.h>
#include <arch/arch.h>
#include <arch/mmu.h>
#include <arch/types.h>
#include <kernel/bootinfo.h>
#include <kernel/pmm.h>
#include <kernel/printk.h>

void kmain(bootinfo_t *boot_info) {
	pmm_init(boot_info);
	printk("Init done.\n");
}
