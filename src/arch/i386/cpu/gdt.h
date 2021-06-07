#include <stdint.h>
#ifndef GDT_H
#define GDT_H

struct gdt_entry {
	/**
	* The first 16 bits of the limit.
	*/
	uint16_t limit_0_15 : 16;
	/** The first 16 bits of the base.
	*/
	uint16_t base_0_15 : 16;
	/** The middle 8 bits of the base.
	*/
	uint8_t base_16_23 : 8;
	/**
	* A (Accessed) - Set to 1 by hardware when the descriptor is accessed and cleared by software.
	*/
	uint8_t accessed : 1;
	/**
	* R (Readable) - If this bit is clear, the segment may be executed but not read from.
	* W (Writable) - If this bit is clear, the segment may be read but not written to.
	*/
	uint8_t readable_or_writable : 1;
	/**
	* C (Conforming) - Code in this segment may be called from less priviliged levels.
	* E (Expand down) - If this bit is clear, the segment expands from base address up to base + limit.
	* If this bit is set, the segment expands from the maximum offset down to limit. Typically used
	* for stack segments.
	*/
	uint8_t conforming_or_expand_down : 1;
	/**
	* T (Type) - If this bit is set, this is a code segment descriptor. If this bit is clear, this is a data/stack segment descriptor, which had "D" replaced by "B", "C" replaced by "E" and "R" replaced by "W".
	* This is infact a special case of the 2-bit type filed, where the preceding bit 12 cleared as "0" refers to more internal system desciptors, for LDT, LSS, and gates.
	*/
	uint8_t type : 1;
	/**
	* S - This bit should be set for code or data segments and should be cleared for systems segments.
	*/
	uint8_t s : 1;
	/** DPL (Descriptor privilege level) - This is a 2 bit field, yielding 4 privilege levels.
	* 00 Highest privilege level, 11 lowest privilege level.
	*/
	uint8_t descriptor_privilege_level : 2;
	/**
	* P (Present) - If this bit is clear, a "segment not present" exception is generated on any reference to this
	* segment.
	*/
	uint8_t present : 1;
	/**
	* The last 4 bits of the limit.
	*/
	uint8_t limit_16_19 : 4;
	/** AVL (Available) - This bit is not used by the hardware and is available for software use.
	*/
	uint8_t available : 1;
	/** This bit is reserved for IA-32 GDT descriptors.
	*/
	uint8_t reserved : 1;
	/** D (Default operand size) - If this bit is clear, this is a 16-bit code segment. If this bit
	* is set, this is a 32 bit segment.
	* B (Big) - If this bit is set, the maximum offset size for a data segment is increased to 32 bit 0xFFFFFFFF. Otherwise it's the 16 bit maximum 0x0000FFFF. Essentially the same meaning as "D".
	*/
	uint8_t default_operand_size_or_big : 1;
	/** G (Granularity) - If this bit is clear, the limit is in units of bytes, with a maximum of 2^20
	* bytes. If this bit is set, the limit is in units of 4096 bytes (4k pages), for a maximum of 2^32 bytes.
	*/
	uint8_t granularity : 1;
	/**
	* The last 8 bits of the base.
	*/
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
					(uint16_t)((limit >> 12) & 0xFFFF), \
					(uint16_t)(base & 0xFFFF), \
					(uint8_t)((base >> 16) & 0xFF), \
					(uint8_t)(accessed), \
					(uint8_t)(readable_or_writable), \
					(uint8_t)(conforming_or_expand_down), \
					(uint8_t)(type), \
					(uint8_t)(s), \
					(uint8_t)((descriptor_privilege_level >> 30) & 0x3), \
					(uint8_t)(present), \
					(uint8_t)(limit >> 28), \
					(uint8_t)(available), \
					(uint8_t)(reserved), \
					(uint8_t)(default_operand_size_or_big), \
					(uint8_t)(granularity), \
					(uint8_t)(base >> 24) \
	}

#define SEGMENT_NULL SEGMENT(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define SEGMENT_KCODE(base, limit) (SEGMENT(base, limit, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1))
#define SEGMENT_KDATA(base, limit) (SEGMENT(base, limit, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1))

#endif /** GDT_H */
