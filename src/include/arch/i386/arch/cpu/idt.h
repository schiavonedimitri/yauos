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
    uint8_t gate_type : 4;
    uint8_t zero : 1;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offset_48_63;
} __attribute__((packed));

typedef struct idt_entry idt_entry_t;

struct idt_descriptor {
	uint16_t table_size;
	idt_entry_t *table_address;
} __attribute__((packed));

typedef struct idt_descriptor idt_descriptor_t;

#endif /** IDT_H */