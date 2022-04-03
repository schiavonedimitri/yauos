#include <arch/cpu/cpu.h>
#include <arch/cpu/gdt.h>

extern cpu_data_t *cpu_data;
extern void load_gdt(gdt_descriptor_t*);
static gdt_descriptor_t gdt_descriptor;

static gdt_entry_t gdt[3] = {
	SEGMENT_NULL,
	SEGMENT_KCODE(0, 0xFFFFFFFF),
	SEGMENT_KDATA(0, 0xFFFFFFFF)
};

void gdt_init() {
	gdt_descriptor.table_size = (sizeof(gdt_entry_t) * 3) - 1;
	gdt_descriptor.table_address = &gdt[0];
	load_gdt(&gdt_descriptor);
}
