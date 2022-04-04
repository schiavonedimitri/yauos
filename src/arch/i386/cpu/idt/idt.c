#include <arch/cpu/idt.h>
#include <lib/string.h>

extern void load_idt(idt_descriptor_t*);

/*
 * Standard i386 cpu exceptions.
 * Defined in isr_low.S
 */
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
extern void isr_err_stub_21(void);
extern void isr_no_err_stub_22(void);
extern void isr_no_err_stub_23(void);
extern void isr_no_err_stub_24(void);
extern void isr_no_err_stub_25(void);
extern void isr_no_err_stub_26(void);
extern void isr_no_err_stub_27(void);
extern void isr_no_err_stub_28(void);
extern void isr_err_stub_29(void);
extern void isr_err_stub_30(void);
extern void isr_no_err_stub_31(void);
extern void isr_no_err_stub_32(void);
extern void isr_no_err_stub_33(void);
extern void isr_no_err_stub_34(void);
extern void isr_no_err_stub_35(void);
extern void isr_no_err_stub_36(void);
extern void isr_no_err_stub_37(void);
extern void isr_no_err_stub_38(void);
extern void isr_no_err_stub_39(void);
extern void isr_no_err_stub_40(void);
extern void isr_no_err_stub_41(void);
extern void isr_no_err_stub_42(void);
extern void isr_no_err_stub_43(void);
extern void isr_no_err_stub_44(void);
extern void isr_no_err_stub_45(void);
extern void isr_no_err_stub_46(void);

static idt_descriptor_t idt_descriptor;

/*
 * Defining a static array of idt entries of 256 entries, which is the maximum number
 * supported on i386.
 */
static idt_entry_t idt[256];

void idt_set_entry(uint8_t num, uint32_t handler, uint8_t type, uint8_t dpl) {
	idt[num].offset_0_15 = handler & 0xFFFF;
	idt[num].segment_selector = GDT_KERNEL_CODE_OFFSET;
	idt[num].reserved = 0x0;
	idt[num].gate_type = type;
	idt[num].zero = 0x0;
	idt[num].dpl = dpl;
	idt[num].present = 0x1;
	idt[num].offset_48_63 = (handler >> 0x10) & 0xFFFF;
}

void idt_init() {
	memset(&idt, 0x0, sizeof(idt_entry_t) * 256);
	idt[0] = IDT_GATE((uint32_t) isr_no_err_stub_0, INTERRUPT_GATE, DPL_KERNEL);
	idt[1] = IDT_GATE((uint32_t) isr_no_err_stub_1, INTERRUPT_GATE, DPL_KERNEL);
	idt[2] = IDT_GATE((uint32_t) isr_no_err_stub_2, INTERRUPT_GATE, DPL_KERNEL);
	idt[3] = IDT_GATE((uint32_t) isr_no_err_stub_3, INTERRUPT_GATE, DPL_KERNEL);
	idt[4] = IDT_GATE((uint32_t) isr_no_err_stub_4, INTERRUPT_GATE, DPL_KERNEL);
	idt[5] = IDT_GATE((uint32_t) isr_no_err_stub_5, INTERRUPT_GATE, DPL_KERNEL);
	idt[6] = IDT_GATE((uint32_t) isr_no_err_stub_6, INTERRUPT_GATE, DPL_KERNEL);
	idt[7] = IDT_GATE((uint32_t) isr_no_err_stub_7, INTERRUPT_GATE, DPL_KERNEL);
	idt[8] = IDT_GATE((uint32_t) isr_err_stub_8, INTERRUPT_GATE, DPL_KERNEL);
	idt[9] = IDT_GATE((uint32_t) isr_no_err_stub_9, INTERRUPT_GATE, DPL_KERNEL);
	idt[10] = IDT_GATE((uint32_t) isr_err_stub_10, INTERRUPT_GATE, DPL_KERNEL);
	idt[11] = IDT_GATE((uint32_t) isr_err_stub_11, INTERRUPT_GATE, DPL_KERNEL);
	idt[12] = IDT_GATE((uint32_t) isr_err_stub_12, INTERRUPT_GATE, DPL_KERNEL);
	idt[13] = IDT_GATE((uint32_t) isr_err_stub_13, INTERRUPT_GATE, DPL_KERNEL);
	idt[14] = IDT_GATE((uint32_t) isr_err_stub_14, INTERRUPT_GATE, DPL_KERNEL);
	idt[15] = IDT_GATE((uint32_t) isr_no_err_stub_15, INTERRUPT_GATE, DPL_KERNEL);
	idt[16] = IDT_GATE((uint32_t) isr_no_err_stub_16, INTERRUPT_GATE, DPL_KERNEL);
	idt[17] = IDT_GATE((uint32_t) isr_err_stub_17, INTERRUPT_GATE, DPL_KERNEL);
	idt[18] = IDT_GATE((uint32_t) isr_no_err_stub_18, INTERRUPT_GATE, DPL_KERNEL);
	idt[19] = IDT_GATE((uint32_t) isr_no_err_stub_19, INTERRUPT_GATE, DPL_KERNEL);
	idt[20] = IDT_GATE((uint32_t) isr_no_err_stub_20, INTERRUPT_GATE, DPL_KERNEL);
	idt[21] = IDT_GATE((uint32_t) isr_err_stub_21, INTERRUPT_GATE, DPL_KERNEL);
	idt[22] = IDT_GATE((uint32_t) isr_no_err_stub_22, INTERRUPT_GATE, DPL_KERNEL);
	idt[23] = IDT_GATE((uint32_t) isr_no_err_stub_23, INTERRUPT_GATE, DPL_KERNEL);
	idt[24] = IDT_GATE((uint32_t) isr_no_err_stub_24, INTERRUPT_GATE, DPL_KERNEL);
	idt[25] = IDT_GATE((uint32_t) isr_no_err_stub_25, INTERRUPT_GATE, DPL_KERNEL);
	idt[26] = IDT_GATE((uint32_t) isr_no_err_stub_26, INTERRUPT_GATE, DPL_KERNEL);
	idt[27] = IDT_GATE((uint32_t) isr_no_err_stub_27, INTERRUPT_GATE, DPL_KERNEL);
	idt[28] = IDT_GATE((uint32_t) isr_no_err_stub_28, INTERRUPT_GATE, DPL_KERNEL);
	idt[29] = IDT_GATE((uint32_t) isr_err_stub_29, INTERRUPT_GATE, DPL_KERNEL);
	idt[30] = IDT_GATE((uint32_t) isr_err_stub_30, INTERRUPT_GATE, DPL_KERNEL);
	idt[31] = IDT_GATE((uint32_t) isr_no_err_stub_31, INTERRUPT_GATE, DPL_KERNEL);
	idt[32] = IDT_GATE((uint32_t) isr_no_err_stub_32, INTERRUPT_GATE, DPL_KERNEL);
	idt[33] = IDT_GATE((uint32_t) isr_no_err_stub_33, INTERRUPT_GATE, DPL_KERNEL);
	idt[34] = IDT_GATE((uint32_t) isr_no_err_stub_34, INTERRUPT_GATE, DPL_KERNEL);
	idt[35] = IDT_GATE((uint32_t) isr_no_err_stub_35, INTERRUPT_GATE, DPL_KERNEL);
	idt[36] = IDT_GATE((uint32_t) isr_no_err_stub_36, INTERRUPT_GATE, DPL_KERNEL);
	idt[37] = IDT_GATE((uint32_t) isr_no_err_stub_37, INTERRUPT_GATE, DPL_KERNEL);
	idt[38] = IDT_GATE((uint32_t) isr_no_err_stub_38, INTERRUPT_GATE, DPL_KERNEL);
	idt[39] = IDT_GATE((uint32_t) isr_no_err_stub_39, INTERRUPT_GATE, DPL_KERNEL);
	idt[40] = IDT_GATE((uint32_t) isr_no_err_stub_40, INTERRUPT_GATE, DPL_KERNEL);
	idt[41] = IDT_GATE((uint32_t) isr_no_err_stub_41, INTERRUPT_GATE, DPL_KERNEL);
	idt[42] = IDT_GATE((uint32_t) isr_no_err_stub_42, INTERRUPT_GATE, DPL_KERNEL);
	idt[43] = IDT_GATE((uint32_t) isr_no_err_stub_43, INTERRUPT_GATE, DPL_KERNEL);
	idt[44] = IDT_GATE((uint32_t) isr_no_err_stub_44, INTERRUPT_GATE, DPL_KERNEL);
	idt[45] = IDT_GATE((uint32_t) isr_no_err_stub_45, INTERRUPT_GATE, DPL_KERNEL);
	idt[46] = IDT_GATE((uint32_t) isr_no_err_stub_46, INTERRUPT_GATE, DPL_KERNEL);
	idt_descriptor.table_size = (sizeof(idt_entry_t) * 256) - 1;
	idt_descriptor.table_address = &idt[0];
	load_idt(&idt_descriptor);
}