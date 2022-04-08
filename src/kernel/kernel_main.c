#include <stdint.h>
#include <arch/cpu/cpu.h>
#include <arch/mmu.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>

void kernel_main(bootinfo_t *boot_info) {
	printk("[KERNEL]: Arch init complete.\n");
	k_malloc_init();
}