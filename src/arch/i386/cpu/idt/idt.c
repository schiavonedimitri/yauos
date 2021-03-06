#include <stdbool.h>
#include <arch/cpu/exception.h>
#include <arch/cpu/idt.h>
#include <arch/cpu/interrupt.h>
#include <lib/string.h>

static idt_descriptor_t idt_descriptor;

/*
 * Defining a static array of idt entries of 256 entries, which is the maximum number
 * supported on i386.
 */

static idt_entry_t idt[256];

void idt_set_entry(uint8_t num, uint32_t handler, uint8_t type, uint8_t dpl) {
	idt[num].offset_0_15 = handler & 0xFFFF;
	idt[num].segment_selector = GDT_KERNEL_CODE_OFFSET;
	idt[num].reserved = 0;
	idt[num].gate_type = type;
	idt[num].zero = 0;
	idt[num].dpl = dpl;
	idt[num].present = 1;
	idt[num].offset_48_63 = (handler >> 16) & 0xFFFF;
}

void idt_init(bool bsp) {
	if (bsp) {
		memset(&idt, 0, sizeof(idt_entry_t) * 256);
		idt[0] = IDT_GATE((uint32_t) exception_no_err_stub_0, TRAP_GATE, DPL_KERNEL);
		idt[1] = IDT_GATE((uint32_t) exception_no_err_stub_1, TRAP_GATE, DPL_KERNEL);
		idt[2] = IDT_GATE((uint32_t) exception_no_err_stub_2, TRAP_GATE, DPL_KERNEL);
		idt[3] = IDT_GATE((uint32_t) exception_no_err_stub_3, TRAP_GATE, DPL_KERNEL);
		idt[4] = IDT_GATE((uint32_t) exception_no_err_stub_4, TRAP_GATE, DPL_KERNEL);
		idt[5] = IDT_GATE((uint32_t) exception_no_err_stub_5, TRAP_GATE, DPL_KERNEL);
		idt[6] = IDT_GATE((uint32_t) exception_no_err_stub_6, TRAP_GATE, DPL_KERNEL);
		idt[7] = IDT_GATE((uint32_t) exception_no_err_stub_7, TRAP_GATE, DPL_KERNEL);
		idt[8] = IDT_GATE((uint32_t) exception_err_stub_8, TRAP_GATE, DPL_KERNEL);
		idt[9] = IDT_GATE((uint32_t) exception_no_err_stub_9, TRAP_GATE, DPL_KERNEL);
		idt[10] = IDT_GATE((uint32_t) exception_err_stub_10, TRAP_GATE, DPL_KERNEL);
		idt[11] = IDT_GATE((uint32_t) exception_err_stub_11, TRAP_GATE, DPL_KERNEL);
		idt[12] = IDT_GATE((uint32_t) exception_err_stub_12, TRAP_GATE, DPL_KERNEL);
		idt[13] = IDT_GATE((uint32_t) exception_err_stub_13, TRAP_GATE, DPL_KERNEL);
		idt[14] = IDT_GATE((uint32_t) exception_err_stub_14, TRAP_GATE, DPL_KERNEL);
		idt[15] = IDT_GATE((uint32_t) exception_no_err_stub_15, TRAP_GATE, DPL_KERNEL);
		idt[16] = IDT_GATE((uint32_t) exception_no_err_stub_16, TRAP_GATE, DPL_KERNEL);
		idt[17] = IDT_GATE((uint32_t) exception_err_stub_17, TRAP_GATE, DPL_KERNEL);
		idt[18] = IDT_GATE((uint32_t) exception_no_err_stub_18, TRAP_GATE, DPL_KERNEL);
		idt[19] = IDT_GATE((uint32_t) exception_no_err_stub_19, TRAP_GATE, DPL_KERNEL);
		idt[20] = IDT_GATE((uint32_t) exception_no_err_stub_20, TRAP_GATE, DPL_KERNEL);
		idt[21] = IDT_GATE((uint32_t) exception_err_stub_21, TRAP_GATE, DPL_KERNEL);
		idt[22] = IDT_GATE((uint32_t) exception_no_err_stub_22, TRAP_GATE, DPL_KERNEL);
		idt[23] = IDT_GATE((uint32_t) exception_no_err_stub_23, TRAP_GATE, DPL_KERNEL);
		idt[24] = IDT_GATE((uint32_t) exception_no_err_stub_24, TRAP_GATE, DPL_KERNEL);
		idt[25] = IDT_GATE((uint32_t) exception_no_err_stub_25, TRAP_GATE, DPL_KERNEL);
		idt[26] = IDT_GATE((uint32_t) exception_no_err_stub_26, TRAP_GATE, DPL_KERNEL);
		idt[27] = IDT_GATE((uint32_t) exception_no_err_stub_27, TRAP_GATE, DPL_KERNEL);
		idt[28] = IDT_GATE((uint32_t) exception_no_err_stub_28, TRAP_GATE, DPL_KERNEL);
		idt[29] = IDT_GATE((uint32_t) exception_err_stub_29, TRAP_GATE, DPL_KERNEL);
		idt[30] = IDT_GATE((uint32_t) exception_err_stub_30, TRAP_GATE, DPL_KERNEL);
		idt[31] = IDT_GATE((uint32_t) exception_no_err_stub_31, TRAP_GATE, DPL_KERNEL);
		idt[32] = IDT_GATE((uint32_t) interrupt_stub_32, INTERRUPT_GATE, DPL_KERNEL);
		idt[33] = IDT_GATE((uint32_t) interrupt_stub_33, INTERRUPT_GATE, DPL_KERNEL);
		idt[34] = IDT_GATE((uint32_t) interrupt_stub_34, INTERRUPT_GATE, DPL_KERNEL);
		idt[35] = IDT_GATE((uint32_t) interrupt_stub_35, INTERRUPT_GATE, DPL_KERNEL);
		idt[36] = IDT_GATE((uint32_t) interrupt_stub_36, INTERRUPT_GATE, DPL_KERNEL);
		idt[37] = IDT_GATE((uint32_t) interrupt_stub_37, INTERRUPT_GATE, DPL_KERNEL);
		idt[38] = IDT_GATE((uint32_t) interrupt_stub_38, INTERRUPT_GATE, DPL_KERNEL);
		idt[39] = IDT_GATE((uint32_t) interrupt_stub_39, INTERRUPT_GATE, DPL_KERNEL);
		idt[40] = IDT_GATE((uint32_t) interrupt_stub_40, INTERRUPT_GATE, DPL_KERNEL);
		idt[41] = IDT_GATE((uint32_t) interrupt_stub_41, INTERRUPT_GATE, DPL_KERNEL);
		idt[42] = IDT_GATE((uint32_t) interrupt_stub_42, INTERRUPT_GATE, DPL_KERNEL);
		idt[43] = IDT_GATE((uint32_t) interrupt_stub_43, INTERRUPT_GATE, DPL_KERNEL);
		idt[44] = IDT_GATE((uint32_t) interrupt_stub_44, INTERRUPT_GATE, DPL_KERNEL);
		idt[45] = IDT_GATE((uint32_t) interrupt_stub_45, INTERRUPT_GATE, DPL_KERNEL);
		idt[46] = IDT_GATE((uint32_t) interrupt_stub_46, INTERRUPT_GATE, DPL_KERNEL);
		idt[47] = IDT_GATE((uint32_t) interrupt_stub_47, INTERRUPT_GATE, DPL_KERNEL);
		idt[48] = IDT_GATE((uint32_t) interrupt_stub_48, INTERRUPT_GATE, DPL_KERNEL);
		idt[49] = IDT_GATE((uint32_t) interrupt_stub_49, INTERRUPT_GATE, DPL_KERNEL);
		idt[50] = IDT_GATE((uint32_t) interrupt_stub_50, INTERRUPT_GATE, DPL_KERNEL);
		idt[51] = IDT_GATE((uint32_t) interrupt_stub_51, INTERRUPT_GATE, DPL_KERNEL);
		idt[52] = IDT_GATE((uint32_t) interrupt_stub_52, INTERRUPT_GATE, DPL_KERNEL);
		idt[53] = IDT_GATE((uint32_t) interrupt_stub_53, INTERRUPT_GATE, DPL_KERNEL);
		idt[54] = IDT_GATE((uint32_t) interrupt_stub_54, INTERRUPT_GATE, DPL_KERNEL);
		idt[55] = IDT_GATE((uint32_t) interrupt_stub_55, INTERRUPT_GATE, DPL_KERNEL);
		idt[56] = IDT_GATE((uint32_t) interrupt_stub_56, INTERRUPT_GATE, DPL_KERNEL);
		idt[57] = IDT_GATE((uint32_t) interrupt_stub_57, INTERRUPT_GATE, DPL_KERNEL);
		idt[58] = IDT_GATE((uint32_t) interrupt_stub_58, INTERRUPT_GATE, DPL_KERNEL);
		idt[59] = IDT_GATE((uint32_t) interrupt_stub_59, INTERRUPT_GATE, DPL_KERNEL);
		idt[60] = IDT_GATE((uint32_t) interrupt_stub_60, INTERRUPT_GATE, DPL_KERNEL);
		idt[61] = IDT_GATE((uint32_t) interrupt_stub_61, INTERRUPT_GATE, DPL_KERNEL);
		idt[62] = IDT_GATE((uint32_t) interrupt_stub_62, INTERRUPT_GATE, DPL_KERNEL);
		idt[63] = IDT_GATE((uint32_t) interrupt_stub_63, INTERRUPT_GATE, DPL_KERNEL);
		idt[64] = IDT_GATE((uint32_t) interrupt_stub_64, INTERRUPT_GATE, DPL_KERNEL);
		idt[65] = IDT_GATE((uint32_t) interrupt_stub_65, INTERRUPT_GATE, DPL_KERNEL);
		idt[66] = IDT_GATE((uint32_t) interrupt_stub_66, INTERRUPT_GATE, DPL_KERNEL);
		idt[67] = IDT_GATE((uint32_t) interrupt_stub_67, INTERRUPT_GATE, DPL_KERNEL);
		idt[68] = IDT_GATE((uint32_t) interrupt_stub_68, INTERRUPT_GATE, DPL_KERNEL);
		idt[69] = IDT_GATE((uint32_t) interrupt_stub_69, INTERRUPT_GATE, DPL_KERNEL);
		idt[70] = IDT_GATE((uint32_t) interrupt_stub_70, INTERRUPT_GATE, DPL_KERNEL);
		idt[71] = IDT_GATE((uint32_t) interrupt_stub_71, INTERRUPT_GATE, DPL_KERNEL);
		idt[72] = IDT_GATE((uint32_t) interrupt_stub_72, INTERRUPT_GATE, DPL_KERNEL);
		idt[73] = IDT_GATE((uint32_t) interrupt_stub_73, INTERRUPT_GATE, DPL_KERNEL);
		idt[74] = IDT_GATE((uint32_t) interrupt_stub_74, INTERRUPT_GATE, DPL_KERNEL);
		idt[75] = IDT_GATE((uint32_t) interrupt_stub_75, INTERRUPT_GATE, DPL_KERNEL);
		idt[76] = IDT_GATE((uint32_t) interrupt_stub_76, INTERRUPT_GATE, DPL_KERNEL);
		idt[77] = IDT_GATE((uint32_t) interrupt_stub_77, INTERRUPT_GATE, DPL_KERNEL);
		idt[78] = IDT_GATE((uint32_t) interrupt_stub_78, INTERRUPT_GATE, DPL_KERNEL);
		idt[79] = IDT_GATE((uint32_t) interrupt_stub_79, INTERRUPT_GATE, DPL_KERNEL);
		idt[80] = IDT_GATE((uint32_t) interrupt_stub_80, INTERRUPT_GATE, DPL_KERNEL);
		idt[81] = IDT_GATE((uint32_t) interrupt_stub_81, INTERRUPT_GATE, DPL_KERNEL);
		idt[82] = IDT_GATE((uint32_t) interrupt_stub_82, INTERRUPT_GATE, DPL_KERNEL);
		idt[83] = IDT_GATE((uint32_t) interrupt_stub_83, INTERRUPT_GATE, DPL_KERNEL);
		idt[84] = IDT_GATE((uint32_t) interrupt_stub_84, INTERRUPT_GATE, DPL_KERNEL);
		idt[85] = IDT_GATE((uint32_t) interrupt_stub_85, INTERRUPT_GATE, DPL_KERNEL);
		idt[86] = IDT_GATE((uint32_t) interrupt_stub_86, INTERRUPT_GATE, DPL_KERNEL);
		idt[87] = IDT_GATE((uint32_t) interrupt_stub_87, INTERRUPT_GATE, DPL_KERNEL);
		idt[88] = IDT_GATE((uint32_t) interrupt_stub_88, INTERRUPT_GATE, DPL_KERNEL);
		idt[89] = IDT_GATE((uint32_t) interrupt_stub_89, INTERRUPT_GATE, DPL_KERNEL);
		idt[90] = IDT_GATE((uint32_t) interrupt_stub_90, INTERRUPT_GATE, DPL_KERNEL);
		idt[91] = IDT_GATE((uint32_t) interrupt_stub_91, INTERRUPT_GATE, DPL_KERNEL);
		idt[92] = IDT_GATE((uint32_t) interrupt_stub_92, INTERRUPT_GATE, DPL_KERNEL);
		idt[93] = IDT_GATE((uint32_t) interrupt_stub_93, INTERRUPT_GATE, DPL_KERNEL);
		idt[94] = IDT_GATE((uint32_t) interrupt_stub_94, INTERRUPT_GATE, DPL_KERNEL);
		idt[95] = IDT_GATE((uint32_t) interrupt_stub_95, INTERRUPT_GATE, DPL_KERNEL);
		idt[96] = IDT_GATE((uint32_t) interrupt_stub_96, INTERRUPT_GATE, DPL_KERNEL);
		idt[97] = IDT_GATE((uint32_t) interrupt_stub_97, INTERRUPT_GATE, DPL_KERNEL);
		idt[98] = IDT_GATE((uint32_t) interrupt_stub_98, INTERRUPT_GATE, DPL_KERNEL);
		idt[99] = IDT_GATE((uint32_t) interrupt_stub_99, INTERRUPT_GATE, DPL_KERNEL);
		idt[100] = IDT_GATE((uint32_t) interrupt_stub_100, INTERRUPT_GATE, DPL_KERNEL);
		idt[101] = IDT_GATE((uint32_t) interrupt_stub_101, INTERRUPT_GATE, DPL_KERNEL);
		idt[102] = IDT_GATE((uint32_t) interrupt_stub_102, INTERRUPT_GATE, DPL_KERNEL);
		idt[103] = IDT_GATE((uint32_t) interrupt_stub_103, INTERRUPT_GATE, DPL_KERNEL);
		idt[104] = IDT_GATE((uint32_t) interrupt_stub_104, INTERRUPT_GATE, DPL_KERNEL);
		idt[105] = IDT_GATE((uint32_t) interrupt_stub_105, INTERRUPT_GATE, DPL_KERNEL);
		idt[106] = IDT_GATE((uint32_t) interrupt_stub_106, INTERRUPT_GATE, DPL_KERNEL);
		idt[107] = IDT_GATE((uint32_t) interrupt_stub_107, INTERRUPT_GATE, DPL_KERNEL);
		idt[108] = IDT_GATE((uint32_t) interrupt_stub_108, INTERRUPT_GATE, DPL_KERNEL);
		idt[109] = IDT_GATE((uint32_t) interrupt_stub_109, INTERRUPT_GATE, DPL_KERNEL);
		idt[110] = IDT_GATE((uint32_t) interrupt_stub_110, INTERRUPT_GATE, DPL_KERNEL);
		idt[111] = IDT_GATE((uint32_t) interrupt_stub_111, INTERRUPT_GATE, DPL_KERNEL);
		idt[112] = IDT_GATE((uint32_t) interrupt_stub_112, INTERRUPT_GATE, DPL_KERNEL);
		idt[113] = IDT_GATE((uint32_t) interrupt_stub_113, INTERRUPT_GATE, DPL_KERNEL);
		idt[114] = IDT_GATE((uint32_t) interrupt_stub_114, INTERRUPT_GATE, DPL_KERNEL);
		idt[115] = IDT_GATE((uint32_t) interrupt_stub_115, INTERRUPT_GATE, DPL_KERNEL);
		idt[116] = IDT_GATE((uint32_t) interrupt_stub_116, INTERRUPT_GATE, DPL_KERNEL);
		idt[117] = IDT_GATE((uint32_t) interrupt_stub_117, INTERRUPT_GATE, DPL_KERNEL);
		idt[118] = IDT_GATE((uint32_t) interrupt_stub_118, INTERRUPT_GATE, DPL_KERNEL);
		idt[119] = IDT_GATE((uint32_t) interrupt_stub_119, INTERRUPT_GATE, DPL_KERNEL);
		idt[120] = IDT_GATE((uint32_t) interrupt_stub_120, INTERRUPT_GATE, DPL_KERNEL);
		idt[121] = IDT_GATE((uint32_t) interrupt_stub_121, INTERRUPT_GATE, DPL_KERNEL);
		idt[122] = IDT_GATE((uint32_t) interrupt_stub_122, INTERRUPT_GATE, DPL_KERNEL);
		idt[123] = IDT_GATE((uint32_t) interrupt_stub_123, INTERRUPT_GATE, DPL_KERNEL);
		idt[124] = IDT_GATE((uint32_t) interrupt_stub_124, INTERRUPT_GATE, DPL_KERNEL);
		idt[125] = IDT_GATE((uint32_t) interrupt_stub_125, INTERRUPT_GATE, DPL_KERNEL);
		idt[126] = IDT_GATE((uint32_t) interrupt_stub_126, INTERRUPT_GATE, DPL_KERNEL);
		idt[127] = IDT_GATE((uint32_t) interrupt_stub_127, INTERRUPT_GATE, DPL_KERNEL);
		idt[128] = IDT_GATE((uint32_t) interrupt_stub_128, INTERRUPT_GATE, DPL_KERNEL);
		idt[129] = IDT_GATE((uint32_t) interrupt_stub_129, INTERRUPT_GATE, DPL_KERNEL);
		idt[130] = IDT_GATE((uint32_t) interrupt_stub_130, INTERRUPT_GATE, DPL_KERNEL);
		idt[131] = IDT_GATE((uint32_t) interrupt_stub_131, INTERRUPT_GATE, DPL_KERNEL);
		idt[132] = IDT_GATE((uint32_t) interrupt_stub_132, INTERRUPT_GATE, DPL_KERNEL);
		idt[133] = IDT_GATE((uint32_t) interrupt_stub_133, INTERRUPT_GATE, DPL_KERNEL);
		idt[134] = IDT_GATE((uint32_t) interrupt_stub_134, INTERRUPT_GATE, DPL_KERNEL);
		idt[135] = IDT_GATE((uint32_t) interrupt_stub_135, INTERRUPT_GATE, DPL_KERNEL);
		idt[136] = IDT_GATE((uint32_t) interrupt_stub_136, INTERRUPT_GATE, DPL_KERNEL);
		idt[137] = IDT_GATE((uint32_t) interrupt_stub_137, INTERRUPT_GATE, DPL_KERNEL);
		idt[138] = IDT_GATE((uint32_t) interrupt_stub_138, INTERRUPT_GATE, DPL_KERNEL);
		idt[139] = IDT_GATE((uint32_t) interrupt_stub_139, INTERRUPT_GATE, DPL_KERNEL);
		idt[140] = IDT_GATE((uint32_t) interrupt_stub_140, INTERRUPT_GATE, DPL_KERNEL);
		idt[141] = IDT_GATE((uint32_t) interrupt_stub_141, INTERRUPT_GATE, DPL_KERNEL);
		idt[142] = IDT_GATE((uint32_t) interrupt_stub_142, INTERRUPT_GATE, DPL_KERNEL);
		idt[143] = IDT_GATE((uint32_t) interrupt_stub_143, INTERRUPT_GATE, DPL_KERNEL);
		idt[144] = IDT_GATE((uint32_t) interrupt_stub_144, INTERRUPT_GATE, DPL_KERNEL);
		idt[145] = IDT_GATE((uint32_t) interrupt_stub_145, INTERRUPT_GATE, DPL_KERNEL);
		idt[146] = IDT_GATE((uint32_t) interrupt_stub_146, INTERRUPT_GATE, DPL_KERNEL);
		idt[147] = IDT_GATE((uint32_t) interrupt_stub_147, INTERRUPT_GATE, DPL_KERNEL);
		idt[148] = IDT_GATE((uint32_t) interrupt_stub_148, INTERRUPT_GATE, DPL_KERNEL);
		idt[149] = IDT_GATE((uint32_t) interrupt_stub_149, INTERRUPT_GATE, DPL_KERNEL);
		idt[150] = IDT_GATE((uint32_t) interrupt_stub_150, INTERRUPT_GATE, DPL_KERNEL);
		idt[151] = IDT_GATE((uint32_t) interrupt_stub_151, INTERRUPT_GATE, DPL_KERNEL);
		idt[152] = IDT_GATE((uint32_t) interrupt_stub_152, INTERRUPT_GATE, DPL_KERNEL);
		idt[153] = IDT_GATE((uint32_t) interrupt_stub_153, INTERRUPT_GATE, DPL_KERNEL);
		idt[154] = IDT_GATE((uint32_t) interrupt_stub_154, INTERRUPT_GATE, DPL_KERNEL);
		idt[155] = IDT_GATE((uint32_t) interrupt_stub_155, INTERRUPT_GATE, DPL_KERNEL);
		idt[156] = IDT_GATE((uint32_t) interrupt_stub_156, INTERRUPT_GATE, DPL_KERNEL);
		idt[157] = IDT_GATE((uint32_t) interrupt_stub_157, INTERRUPT_GATE, DPL_KERNEL);
		idt[158] = IDT_GATE((uint32_t) interrupt_stub_158, INTERRUPT_GATE, DPL_KERNEL);
		idt[159] = IDT_GATE((uint32_t) interrupt_stub_159, INTERRUPT_GATE, DPL_KERNEL);
		idt[160] = IDT_GATE((uint32_t) interrupt_stub_160, INTERRUPT_GATE, DPL_KERNEL);
		idt[161] = IDT_GATE((uint32_t) interrupt_stub_161, INTERRUPT_GATE, DPL_KERNEL);
		idt[162] = IDT_GATE((uint32_t) interrupt_stub_162, INTERRUPT_GATE, DPL_KERNEL);
		idt[163] = IDT_GATE((uint32_t) interrupt_stub_163, INTERRUPT_GATE, DPL_KERNEL);
		idt[164] = IDT_GATE((uint32_t) interrupt_stub_164, INTERRUPT_GATE, DPL_KERNEL);
		idt[165] = IDT_GATE((uint32_t) interrupt_stub_165, INTERRUPT_GATE, DPL_KERNEL);
		idt[166] = IDT_GATE((uint32_t) interrupt_stub_166, INTERRUPT_GATE, DPL_KERNEL);
		idt[167] = IDT_GATE((uint32_t) interrupt_stub_167, INTERRUPT_GATE, DPL_KERNEL);
		idt[168] = IDT_GATE((uint32_t) interrupt_stub_168, INTERRUPT_GATE, DPL_KERNEL);
		idt[169] = IDT_GATE((uint32_t) interrupt_stub_169, INTERRUPT_GATE, DPL_KERNEL);
		idt[170] = IDT_GATE((uint32_t) interrupt_stub_170, INTERRUPT_GATE, DPL_KERNEL);
		idt[171] = IDT_GATE((uint32_t) interrupt_stub_171, INTERRUPT_GATE, DPL_KERNEL);
		idt[172] = IDT_GATE((uint32_t) interrupt_stub_172, INTERRUPT_GATE, DPL_KERNEL);
		idt[173] = IDT_GATE((uint32_t) interrupt_stub_173, INTERRUPT_GATE, DPL_KERNEL);
		idt[174] = IDT_GATE((uint32_t) interrupt_stub_174, INTERRUPT_GATE, DPL_KERNEL);
		idt[175] = IDT_GATE((uint32_t) interrupt_stub_175, INTERRUPT_GATE, DPL_KERNEL);
		idt[176] = IDT_GATE((uint32_t) interrupt_stub_176, INTERRUPT_GATE, DPL_KERNEL);
		idt[177] = IDT_GATE((uint32_t) interrupt_stub_177, INTERRUPT_GATE, DPL_KERNEL);
		idt[178] = IDT_GATE((uint32_t) interrupt_stub_178, INTERRUPT_GATE, DPL_KERNEL);
		idt[179] = IDT_GATE((uint32_t) interrupt_stub_179, INTERRUPT_GATE, DPL_KERNEL);
		idt[180] = IDT_GATE((uint32_t) interrupt_stub_180, INTERRUPT_GATE, DPL_KERNEL);
		idt[181] = IDT_GATE((uint32_t) interrupt_stub_181, INTERRUPT_GATE, DPL_KERNEL);
		idt[182] = IDT_GATE((uint32_t) interrupt_stub_182, INTERRUPT_GATE, DPL_KERNEL);
		idt[183] = IDT_GATE((uint32_t) interrupt_stub_183, INTERRUPT_GATE, DPL_KERNEL);
		idt[184] = IDT_GATE((uint32_t) interrupt_stub_184, INTERRUPT_GATE, DPL_KERNEL);
		idt[185] = IDT_GATE((uint32_t) interrupt_stub_185, INTERRUPT_GATE, DPL_KERNEL);
		idt[186] = IDT_GATE((uint32_t) interrupt_stub_186, INTERRUPT_GATE, DPL_KERNEL);
		idt[187] = IDT_GATE((uint32_t) interrupt_stub_187, INTERRUPT_GATE, DPL_KERNEL);
		idt[188] = IDT_GATE((uint32_t) interrupt_stub_188, INTERRUPT_GATE, DPL_KERNEL);
		idt[189] = IDT_GATE((uint32_t) interrupt_stub_189, INTERRUPT_GATE, DPL_KERNEL);
		idt[190] = IDT_GATE((uint32_t) interrupt_stub_190, INTERRUPT_GATE, DPL_KERNEL);
		idt[191] = IDT_GATE((uint32_t) interrupt_stub_191, INTERRUPT_GATE, DPL_KERNEL);
		idt[192] = IDT_GATE((uint32_t) interrupt_stub_192, INTERRUPT_GATE, DPL_KERNEL);
		idt[193] = IDT_GATE((uint32_t) interrupt_stub_193, INTERRUPT_GATE, DPL_KERNEL);
		idt[194] = IDT_GATE((uint32_t) interrupt_stub_194, INTERRUPT_GATE, DPL_KERNEL);
		idt[195] = IDT_GATE((uint32_t) interrupt_stub_195, INTERRUPT_GATE, DPL_KERNEL);
		idt[196] = IDT_GATE((uint32_t) interrupt_stub_196, INTERRUPT_GATE, DPL_KERNEL);
		idt[197] = IDT_GATE((uint32_t) interrupt_stub_197, INTERRUPT_GATE, DPL_KERNEL);
		idt[198] = IDT_GATE((uint32_t) interrupt_stub_198, INTERRUPT_GATE, DPL_KERNEL);
		idt[199] = IDT_GATE((uint32_t) interrupt_stub_199, INTERRUPT_GATE, DPL_KERNEL);
		idt[200] = IDT_GATE((uint32_t) interrupt_stub_200, INTERRUPT_GATE, DPL_KERNEL);
		idt[201] = IDT_GATE((uint32_t) interrupt_stub_201, INTERRUPT_GATE, DPL_KERNEL);
		idt[202] = IDT_GATE((uint32_t) interrupt_stub_202, INTERRUPT_GATE, DPL_KERNEL);
		idt[203] = IDT_GATE((uint32_t) interrupt_stub_203, INTERRUPT_GATE, DPL_KERNEL);
		idt[204] = IDT_GATE((uint32_t) interrupt_stub_204, INTERRUPT_GATE, DPL_KERNEL);
		idt[205] = IDT_GATE((uint32_t) interrupt_stub_205, INTERRUPT_GATE, DPL_KERNEL);
		idt[206] = IDT_GATE((uint32_t) interrupt_stub_206, INTERRUPT_GATE, DPL_KERNEL);
		idt[207] = IDT_GATE((uint32_t) interrupt_stub_207, INTERRUPT_GATE, DPL_KERNEL);
		idt[208] = IDT_GATE((uint32_t) interrupt_stub_208, INTERRUPT_GATE, DPL_KERNEL);
		idt[209] = IDT_GATE((uint32_t) interrupt_stub_209, INTERRUPT_GATE, DPL_KERNEL);
		idt[210] = IDT_GATE((uint32_t) interrupt_stub_210, INTERRUPT_GATE, DPL_KERNEL);
		idt[211] = IDT_GATE((uint32_t) interrupt_stub_211, INTERRUPT_GATE, DPL_KERNEL);
		idt[212] = IDT_GATE((uint32_t) interrupt_stub_212, INTERRUPT_GATE, DPL_KERNEL);
		idt[213] = IDT_GATE((uint32_t) interrupt_stub_213, INTERRUPT_GATE, DPL_KERNEL);
		idt[214] = IDT_GATE((uint32_t) interrupt_stub_214, INTERRUPT_GATE, DPL_KERNEL);
		idt[215] = IDT_GATE((uint32_t) interrupt_stub_215, INTERRUPT_GATE, DPL_KERNEL);
		idt[216] = IDT_GATE((uint32_t) interrupt_stub_216, INTERRUPT_GATE, DPL_KERNEL);
		idt[217] = IDT_GATE((uint32_t) interrupt_stub_217, INTERRUPT_GATE, DPL_KERNEL);
		idt[218] = IDT_GATE((uint32_t) interrupt_stub_218, INTERRUPT_GATE, DPL_KERNEL);
		idt[219] = IDT_GATE((uint32_t) interrupt_stub_219, INTERRUPT_GATE, DPL_KERNEL);
		idt[220] = IDT_GATE((uint32_t) interrupt_stub_220, INTERRUPT_GATE, DPL_KERNEL);
		idt[221] = IDT_GATE((uint32_t) interrupt_stub_221, INTERRUPT_GATE, DPL_KERNEL);
		idt[222] = IDT_GATE((uint32_t) interrupt_stub_222, INTERRUPT_GATE, DPL_KERNEL);
		idt[223] = IDT_GATE((uint32_t) interrupt_stub_223, INTERRUPT_GATE, DPL_KERNEL);
		idt[224] = IDT_GATE((uint32_t) interrupt_stub_224, INTERRUPT_GATE, DPL_KERNEL);
		idt[225] = IDT_GATE((uint32_t) interrupt_stub_225, INTERRUPT_GATE, DPL_KERNEL);
		idt[226] = IDT_GATE((uint32_t) interrupt_stub_226, INTERRUPT_GATE, DPL_KERNEL);
		idt[227] = IDT_GATE((uint32_t) interrupt_stub_227, INTERRUPT_GATE, DPL_KERNEL);
		idt[228] = IDT_GATE((uint32_t) interrupt_stub_228, INTERRUPT_GATE, DPL_KERNEL);
		idt[229] = IDT_GATE((uint32_t) interrupt_stub_229, INTERRUPT_GATE, DPL_KERNEL);
		idt[230] = IDT_GATE((uint32_t) interrupt_stub_230, INTERRUPT_GATE, DPL_KERNEL);
		idt[231] = IDT_GATE((uint32_t) interrupt_stub_231, INTERRUPT_GATE, DPL_KERNEL);
		idt[232] = IDT_GATE((uint32_t) interrupt_stub_232, INTERRUPT_GATE, DPL_KERNEL);
		idt[233] = IDT_GATE((uint32_t) interrupt_stub_233, INTERRUPT_GATE, DPL_KERNEL);
		idt[234] = IDT_GATE((uint32_t) interrupt_stub_234, INTERRUPT_GATE, DPL_KERNEL);
		idt[235] = IDT_GATE((uint32_t) interrupt_stub_235, INTERRUPT_GATE, DPL_KERNEL);
		idt[236] = IDT_GATE((uint32_t) interrupt_stub_236, INTERRUPT_GATE, DPL_KERNEL);
		idt[237] = IDT_GATE((uint32_t) interrupt_stub_237, INTERRUPT_GATE, DPL_KERNEL);
		idt[238] = IDT_GATE((uint32_t) interrupt_stub_238, INTERRUPT_GATE, DPL_KERNEL);
		idt[239] = IDT_GATE((uint32_t) interrupt_stub_239, INTERRUPT_GATE, DPL_KERNEL);
		idt[240] = IDT_GATE((uint32_t) interrupt_stub_240, INTERRUPT_GATE, DPL_KERNEL);
		idt[241] = IDT_GATE((uint32_t) interrupt_stub_241, INTERRUPT_GATE, DPL_KERNEL);
		idt[242] = IDT_GATE((uint32_t) interrupt_stub_242, INTERRUPT_GATE, DPL_KERNEL);
		idt[243] = IDT_GATE((uint32_t) interrupt_stub_243, INTERRUPT_GATE, DPL_KERNEL);
		idt[244] = IDT_GATE((uint32_t) interrupt_stub_244, INTERRUPT_GATE, DPL_KERNEL);
		idt[245] = IDT_GATE((uint32_t) interrupt_stub_245, INTERRUPT_GATE, DPL_KERNEL);
		idt[246] = IDT_GATE((uint32_t) interrupt_stub_246, INTERRUPT_GATE, DPL_KERNEL);
		idt[247] = IDT_GATE((uint32_t) interrupt_stub_247, INTERRUPT_GATE, DPL_KERNEL);
		idt[248] = IDT_GATE((uint32_t) interrupt_stub_248, INTERRUPT_GATE, DPL_KERNEL);
		idt[249] = IDT_GATE((uint32_t) interrupt_stub_249, INTERRUPT_GATE, DPL_KERNEL);
		idt[250] = IDT_GATE((uint32_t) interrupt_stub_250, INTERRUPT_GATE, DPL_KERNEL);
		idt[251] = IDT_GATE((uint32_t) interrupt_stub_251, INTERRUPT_GATE, DPL_KERNEL);
		idt[252] = IDT_GATE((uint32_t) interrupt_stub_252, INTERRUPT_GATE, DPL_KERNEL);
		idt[253] = IDT_GATE((uint32_t) interrupt_stub_253, INTERRUPT_GATE, DPL_KERNEL);
		idt[254] = IDT_GATE((uint32_t) interrupt_stub_254, INTERRUPT_GATE, DPL_KERNEL);
		idt[255] = IDT_GATE((uint32_t) interrupt_stub_255, INTERRUPT_GATE, DPL_KERNEL);
		idt_descriptor.table_size = (sizeof(idt_entry_t) * 256) - 1;
		idt_descriptor.table_address = &idt[0];
	}
	load_idt(&idt_descriptor);
}