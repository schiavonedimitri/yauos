#include <stdint.h>
#include <arch/mmu.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>

void kernel_main(bootinfo_t *boot_info) {
	printk("[KERNEL]: Arch init complete.\n");
}