#include <stdint.h>
#include <arch/defs.h>
#include <arch/gdt.h>
#include <arch/mmu.h>
#include <arch/multiboot2.h>
#include <arch/pmm.h>
#include <arch/vga.h>
#include <kernel/printk.h>

void kernel_main(uint32_t magic, multiboot2_information_header_t *multiboot2_info) {
	vga_initialize();
	if (magic != MULTIBOOT2_MAGIC) {
		printk("This kernel needs to be loaded by a Multiboot2 compliant bootloader!\n");
		return;
	}
	gdt_init();
	pmm_init(multiboot2_info);
	printk("Initialization done.\n");
}
