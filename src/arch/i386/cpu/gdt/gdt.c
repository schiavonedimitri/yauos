#include <arch/cpu/cpu.h>
#include <arch/cpu/gdt.h>
#include <lib/string.h>

extern cpu_data_t *cpu_data;
extern void load_gdt(gdt_descriptor_t*);
static gdt_descriptor_t gdt_descriptor;

/*
 * Defining a static 8192 array of gdt entries as such is the maximum supported on i386.
 * Some slots will be added later to support per-cpu variables through the use of es, fs, gs 
 * segments.
 */

static gdt_entry_t gdt[8192];

/*
 * This function is used to set new gdt entries after initialization. 
 * This function shuould not be used to alter existing entries that are in use or potential crashes could happen.
 * For now this is used to add entries to support per cpu variables through the use of es, fs, gs segment selectors
 * in smp systems.
 */

void set_gdt_entry(uint16_t number, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {

}

void gdt_init() {
	memset(&gdt, 0x0, sizeof(gdt_entry_t) * 8192);
	// Add basic entries
	gdt[0] = SEGMENT_NULL;
	gdt[1] = SEGMENT_KCODE(0x0, 0xFFFFFFFF);
	gdt[2] = SEGMENT_KDATA(0x0, 0xFFFFFFFF);
	gdt_descriptor.table_size = (sizeof(gdt_entry_t) * 0x3) - 0x1;
	gdt_descriptor.table_address = &gdt[0];
	load_gdt(&gdt_descriptor);
}
