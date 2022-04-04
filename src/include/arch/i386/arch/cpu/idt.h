#ifndef IDT_H
#define IDT_H

#include <arch/cpu/gdt.h>
#include <stdint.h>

#define INTERRUPT_GATE 0xE
#define TRAP_GATE 0xF
#define DPL_USER 0x3
#define DPL_KERNEL 0x0

struct idt_entry {
    uint16_t offset_0_15;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t gate_type : 0x4;
    uint8_t zero : 0x1;
    uint8_t dpl : 0x2;
    uint8_t present : 0x1;
    uint16_t offset_48_63 : 0x10;
} __attribute__((packed));

typedef struct idt_entry idt_entry_t;

struct idt_descriptor {
	uint16_t table_size;
	idt_entry_t *table_address;
} __attribute__((packed));

typedef struct idt_descriptor idt_descriptor_t;

/*
 * Macro used for local definition of gates in initialization code to avoid multiple function calls per gate setup.
 */

#define IDT_GATE(handler, type, dpl) \
	(idt_entry_t) { \
					(uint16_t) (handler & 0xFFFF), \
					(uint16_t) (GDT_KERNEL_CODE_OFFSET), \
					(uint8_t) (0x0), \
					(uint8_t) (type), \
					(uint8_t) (0x0), \
					(uint8_t) (dpl), \
					(uint8_t) (0x1), \
					(uint16_t) ((handler >> 0x10) & 0xFFFF) \
	}

/*
 * This function is used for addding/removing/modifying idt entries at runtime.
 * Mostly used after initialization by the kernel interrupt registration facilities for drivers.
 */

void idt_set_entry(uint8_t, uint32_t, uint8_t, uint8_t);

#endif /** IDT_H */