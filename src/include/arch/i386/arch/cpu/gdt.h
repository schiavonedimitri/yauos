#ifndef GDT_H
#define GDT_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif

#define GDT_KERNEL_CODE_OFFSET 0x08
#define GDT_KERNEL_DATA_OFFSET 0x10
#define GDT_CPU_DATA_OFFSET 0x18
#define GDT_USER_CODE_OFFSET 0x20
#define GDT_USER_DATA_OFFSET 0x28

/* 
 * GDT definitions:
 */

#define GDT_CODE_SEGMENT_NOT_READABLE 0x0
#define GDT_CODE_SEGMENT_READABLE (1 << 1)
#define GDT_DATA_SEGMENT_NOT_WRITEABLE 0x0
#define GDT_DATA_SEGMENT_WRITEABLE (1 << 1)
#define GDT_DATA_SEGMENT_GROW_UP 0x0
#define GDT_DATA_SEGMENT_GROW_DOWN (1 << 2)
#define GDT_CODE_SEGMENT_CONFORMING 0x0
#define GDT_CODE_SEGMENT_NOT_CONFORMING (1 << 2)
#define GDT_DATA_SEGMENT 0x0
#define GDT_CODE_SEGMENT (1 << 3)
#define GDT_SYSTEM_SEGMENT 0x0
#define GDT_NON_SYSTEM_SEGMENT ( 1 << 4)
#define GDT_SEGMENT_DPL_0 0x0
#define GDT_SEGMENT_DPL_1 (1 << 5)
#define GDT_SEGMENT_DPL_2 (2 << 4)
#define GDT_SEGMENT_DPL_3 (3 << 4)
#define GDT_SEGMENT_NOT_PRESENT 0x0
#define GDT_SEGMENT_PRESENT (1 << 7)
#define GDT_FLAGS_GRANULARITY_BYTE 0x0
#define GDT_FLAGS_GRANULARITY_PAGE (1 << 3)
#define GDT_FLAGS_16_BIT_SEGMENT 0x0
#define GDT_FLAGS_32_BIT_SEGMENT (1 << 2)

#ifndef __ASSEMBLER__

struct gdt_entry {
	uint16_t limit_0_15;
	uint16_t base_0_15;
	uint8_t base_16_23;
	uint8_t accessed : 1;
	uint8_t readable_or_writable : 1;
	uint8_t conforming_or_expand_down : 1;
	uint8_t type : 1;
	uint8_t s : 1;
	uint8_t descriptor_privilege_level : 2;
	uint8_t present : 1;
	uint8_t limit_16_19 : 4;
	uint8_t available : 1;
	uint8_t reserved : 1;
	uint8_t default_operand_size_or_big : 1;
	uint8_t granularity : 1;
	uint8_t base_24_31 : 8;
} __attribute__((packed));

typedef struct gdt_entry gdt_entry_t;

struct gdt_descriptor {
	uint16_t table_size;
	gdt_entry_t *table_address;
} __attribute__((packed));

typedef struct gdt_descriptor gdt_descriptor_t;

#define SEGMENT(base, limit, accessed, readable_or_writable, conforming_or_expand_down, type, s, descriptor_privilege_level, present, available, reserved, default_operand_size_or_big, granularity) \
	(gdt_entry_t) { \
					(uint16_t) ((limit >> 12) & 0xFFFF), \
					(uint16_t) (base & 0xFFFF), \
					(uint8_t) ((base >> 16) & 0xFF), \
					(uint8_t) (accessed), \
					(uint8_t) (readable_or_writable), \
					(uint8_t) (conforming_or_expand_down), \
					(uint8_t) (type), \
					(uint8_t) (s), \
					(uint8_t) ((descriptor_privilege_level >> 30) & 0x3), \
					(uint8_t) (present), \
					(uint8_t) (limit >> 28), \
					(uint8_t) (available), \
					(uint8_t) (reserved), \
					(uint8_t) (default_operand_size_or_big), \
					(uint8_t) (granularity), \
					(uint8_t) (base >> 24) \
	}

/*
 * Macros for local definitions of segments (used in the initial gdt setup to avoid function calls per gdt entry.)
 */

#define SEGMENT_NULL SEGMENT(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define SEGMENT_KCODE(base, limit) (SEGMENT(base, limit, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1))
#define SEGMENT_KDATA(base, limit) (SEGMENT(base, limit, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1))
#define SEGMENT_CPU_DATA(base, limit) (SEGMENT(base, limit, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1))

/*
 ^ Used for adding gdt entries at run time after initialization. Used for supporting per cpu variables
 * in smp systems through the use of es, fs, gs segment selectors.
 * Warning: modifying segments in use after initialization at run time might trigger a kernel crash.
 */

void set_gdt_entry(uint16_t , uint32_t, uint32_t, uint8_t, uint8_t);

#endif /** __ASSEMBLER__ */

#endif /** GDT_H */
