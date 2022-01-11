#include <stdint.h>
#include <arch/arch.h>
#include <arch/types.h>
#include <kernel/bootinfo.h>
#include <kernel/pmm.h>
#include <kernel/printk.h>

void kmain(bootinfo_t *boot_info) {
	pmm_init(boot_info);
	printk("Init done.\n");
	phys_addr_t frame;
	while ((frame = pmm_get_free_frame()) < (phys_addr_t) (0xB0000)) {
		printk("PMM: got free frame at: %x\n", frame);
	}
}
