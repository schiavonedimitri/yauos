#include <arch/cpu/idt.h>
#include <lib/string.h>

extern void load_idt(idt_descriptor_t*);
extern void isr_no_err_stub_0(void);
extern void isr_no_err_stub_1(void);
extern void isr_no_err_stub_2(void);
extern void isr_no_err_stub_3(void);
extern void isr_no_err_stub_4(void);
extern void isr_no_err_stub_5(void);
extern void isr_no_err_stub_6(void);
extern void isr_no_err_stub_7(void);
extern void isr_err_stub_8(void);
extern void isr_no_err_stub_9(void);
extern void isr_err_stub_10(void);
extern void isr_err_stub_11(void);
extern void isr_err_stub_12(void);
extern void isr_err_stub_13(void);
extern void isr_err_stub_14(void);
extern void isr_no_err_stub_15(void);
extern void isr_no_err_stub_16(void);
extern void isr_err_stub_17(void);
extern void isr_no_err_stub_18(void);
extern void isr_no_err_stub_19(void);
extern void isr_no_err_stub_20(void);
extern void isr_no_err_stub_21(void);
extern void isr_no_err_stub_22(void);
extern void isr_no_err_stub_23(void);
extern void isr_no_err_stub_24(void);
extern void isr_no_err_stub_25(void);
extern void isr_no_err_stub_26(void);
extern void isr_no_err_stub_27(void);
extern void isr_no_err_stub_28(void);
extern void isr_no_err_stub_29(void);
extern void isr_err_stub_30(void);
extern void isr_no_err_stub_31(void);

static idt_descriptor_t idt_descriptor;
static idt_entry_t idt[256];

static void idt_set_entry(uint8_t num, uint32_t handler, uint8_t type, uint8_t dpl) {
	idt[num].offset_0_15 = handler & 0xFFFF;
	idt[num].segment_selector = GDT_KERNEL_CODE_OFFSET;
	idt[num].reserved = 0x0;
	idt[num].gate_type = type;
	idt[num].zero = 0x0;
	idt[num].dpl = dpl;
	idt[num].present = 0x1;
	idt[num].offset_48_63 = (handler >> 16) & 0xFFFF;
}

void idt_init() {
	memset(&idt, 0x0, sizeof(idt_entry_t) * 256);
	idt_set_entry(0, (uint32_t) isr_no_err_stub_0, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(1, (uint32_t) isr_no_err_stub_1, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(2, (uint32_t) isr_no_err_stub_2, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(3, (uint32_t) isr_no_err_stub_3, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(4, (uint32_t) isr_no_err_stub_4, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(5, (uint32_t) isr_no_err_stub_5, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(6, (uint32_t) isr_no_err_stub_6, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(7, (uint32_t) isr_no_err_stub_7, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(8, (uint32_t) isr_err_stub_8, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(9, (uint32_t) isr_no_err_stub_9, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(10, (uint32_t) isr_err_stub_10, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(11, (uint32_t) isr_err_stub_11, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(12, (uint32_t) isr_err_stub_12, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(13, (uint32_t) isr_err_stub_13, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(14, (uint32_t) isr_err_stub_14, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(15, (uint32_t) isr_no_err_stub_15, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(16, (uint32_t) isr_no_err_stub_16, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(17, (uint32_t) isr_err_stub_17, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(18, (uint32_t) isr_no_err_stub_18, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(19, (uint32_t) isr_no_err_stub_19, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(20, (uint32_t) isr_no_err_stub_20, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(21, (uint32_t) isr_no_err_stub_21, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(22, (uint32_t) isr_no_err_stub_22, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(23, (uint32_t) isr_no_err_stub_23, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(24, (uint32_t) isr_no_err_stub_24, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(25, (uint32_t) isr_no_err_stub_25, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(26, (uint32_t) isr_no_err_stub_26, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(27, (uint32_t) isr_no_err_stub_27, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(28, (uint32_t) isr_no_err_stub_28, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(29, (uint32_t) isr_no_err_stub_29, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(30, (uint32_t) isr_err_stub_30, INTERRUPT_GATE, DPL_KERNEL);
	idt_set_entry(31, (uint32_t) isr_no_err_stub_31, INTERRUPT_GATE, DPL_KERNEL);
	idt_descriptor.table_size = (sizeof(idt_entry_t) * 256) - 1;
	idt_descriptor.table_address = &idt[0];
	load_idt(&idt_descriptor);
}