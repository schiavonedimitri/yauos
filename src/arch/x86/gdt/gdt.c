#include "gdt.h"

extern void load_gdt(gdt_descriptor_t*);
gdt_descriptor_t gdt_descriptor;

gdt_entry_t gdt[3] = {
	SEGMENT_NULL,
	SEGMENT_KCODE(0, 0xFFFFFFFF),
	SEGMENT_KDATA(0, 0xFFFFFFFF)
};

void gdt_init() {
	gdt_descriptor.table_size = (sizeof(gdt_entry_t) * 3) - 1;
	gdt_descriptor.table_address = &gdt[0];
	load_gdt(&gdt_descriptor);
}
