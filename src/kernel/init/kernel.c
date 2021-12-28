#include <stdint.h>
#include <arch/defs.h>
#include <arch/gdt.h>
#include <arch/mmu.h>
#include <arch/multiboot2.h>
#include <arch/pmm.h>
#include <arch/vga.h>
#include <kernel/printk.h>

extern void load_gdt(gdt_descriptor_t*);
gdt_descriptor_t gdt_descriptor;

gdt_entry_t gdt[3] = {
	SEGMENT_NULL,
	SEGMENT_KCODE(0, 0xFFFFFFFF),
	SEGMENT_KDATA(0, 0xFFFFFFFF)
};

void kernel_main(uint32_t magic, multiboot2_information_header_t *multiboot2_info) {
	vga_initialize();
	if (magic != MULTIBOOT2_MAGIC) {
		printk("This kernel needs to be loaded by a Multiboot2 compliant bootloader!\n");
		return;
	}
	gdt_descriptor.table_size = (sizeof(gdt_entry_t) * 3) - 1;
	gdt_descriptor.table_address = &gdt[0];
	load_gdt(&gdt_descriptor);
	pmm_init(multiboot2_info);
}
