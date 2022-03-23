#include <stdalign.h>
#include <stdint.h>
#include <arch/align.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>
#include <kernel/mm/pm.h>

void kernel_main(bootinfo_t *boot_info) {
	for (;;) {
		phys_addr_t frame = get_free_frame();
		if (frame != (phys_addr_t) -1 && frame <= (phys_addr_t) 0x100000) {
			printk("Allocated frame at: %x\n", frame);
		}
		else {
			break;
		}
	}
	asm volatile("int $0x0;");
	arch_halt();
}