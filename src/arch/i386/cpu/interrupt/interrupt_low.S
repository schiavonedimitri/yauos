#include <arch/cpu/gdt.h>

.extern interrupt_common_handler
.type interrupt_common_handler, @function

#define interrupt_stub(num) \
        .global interrupt_stub_ ## num ;\
        .type interrupt_stub_ ## num , @function ;\
        interrupt_stub_ ## num: ;\
                pushl $0 ;\
                pushl $num ;\
                jmp interrupt_common_stub ;\
        .size interrupt_stub_ ## num , . - interrupt_stub_ ## num


.section .text

        # Interrupt stubs.

        interrupt_stub(32)
        interrupt_stub(33)
        interrupt_stub(34)
        interrupt_stub(35)
        interrupt_stub(36)
        interrupt_stub(37)
        interrupt_stub(38)
        interrupt_stub(39)
        interrupt_stub(40)
        interrupt_stub(41)
        interrupt_stub(42)
        interrupt_stub(43)
        interrupt_stub(44)
        interrupt_stub(45)
        interrupt_stub(46)
        interrupt_stub(47)
        interrupt_stub(48)
        interrupt_stub(49)
        interrupt_stub(50)
        interrupt_stub(51)
        interrupt_stub(52)
        interrupt_stub(53)
        interrupt_stub(54)
        interrupt_stub(55)
        interrupt_stub(56)
        interrupt_stub(57)
        interrupt_stub(58)
        interrupt_stub(59)
        interrupt_stub(60)
        interrupt_stub(61)
        interrupt_stub(62)
        interrupt_stub(63)
        interrupt_stub(64)
        interrupt_stub(65)
        interrupt_stub(66)
        interrupt_stub(67)
        interrupt_stub(68)
        interrupt_stub(69)
        interrupt_stub(70)
        interrupt_stub(71)
        interrupt_stub(72)
        interrupt_stub(73)
        interrupt_stub(74)
        interrupt_stub(75)
        interrupt_stub(76)
        interrupt_stub(77)
        interrupt_stub(78)
        interrupt_stub(79)
        interrupt_stub(80)
        interrupt_stub(81)
        interrupt_stub(82)
        interrupt_stub(83)
        interrupt_stub(84)
        interrupt_stub(85)
        interrupt_stub(86)
        interrupt_stub(87)
        interrupt_stub(88)
        interrupt_stub(89)
        interrupt_stub(90)
        interrupt_stub(91)
        interrupt_stub(92)
        interrupt_stub(93)
        interrupt_stub(94)
        interrupt_stub(95)
        interrupt_stub(96)
        interrupt_stub(97)
        interrupt_stub(98)
        interrupt_stub(99)
        interrupt_stub(100)
        interrupt_stub(101)
        interrupt_stub(102)
        interrupt_stub(103)
        interrupt_stub(104)
        interrupt_stub(105)
        interrupt_stub(106)
        interrupt_stub(107)
        interrupt_stub(108)
        interrupt_stub(109)
        interrupt_stub(110)
        interrupt_stub(111)
        interrupt_stub(112)
        interrupt_stub(113)
        interrupt_stub(114)
        interrupt_stub(115)
        interrupt_stub(116)
        interrupt_stub(117)
        interrupt_stub(118)
        interrupt_stub(119)
        interrupt_stub(120)
        interrupt_stub(121)
        interrupt_stub(122)
        interrupt_stub(123)
        interrupt_stub(124)
        interrupt_stub(125)
        interrupt_stub(126)
        interrupt_stub(127)
        interrupt_stub(128)
        interrupt_stub(129)
        interrupt_stub(130)
        interrupt_stub(131)
        interrupt_stub(132)
        interrupt_stub(133)
        interrupt_stub(134)
        interrupt_stub(135)
        interrupt_stub(136)
        interrupt_stub(137)
        interrupt_stub(138)
        interrupt_stub(139)
        interrupt_stub(140)
        interrupt_stub(141)
        interrupt_stub(142)
        interrupt_stub(143)
        interrupt_stub(144)
        interrupt_stub(145)
        interrupt_stub(146)
        interrupt_stub(147)
        interrupt_stub(148)
        interrupt_stub(149)
        interrupt_stub(150)
        interrupt_stub(151)
        interrupt_stub(152)
        interrupt_stub(153)
        interrupt_stub(154)
        interrupt_stub(155)
        interrupt_stub(156)
        interrupt_stub(157)
        interrupt_stub(158)
        interrupt_stub(159)
        interrupt_stub(160)
        interrupt_stub(161)
        interrupt_stub(162)
        interrupt_stub(163)
        interrupt_stub(164)
        interrupt_stub(165)
        interrupt_stub(166)
        interrupt_stub(167)
        interrupt_stub(168)
        interrupt_stub(169)
        interrupt_stub(170)
        interrupt_stub(171)
        interrupt_stub(172)
        interrupt_stub(173)
        interrupt_stub(174)
        interrupt_stub(175)
        interrupt_stub(176)
        interrupt_stub(177)
        interrupt_stub(178)
        interrupt_stub(179)
        interrupt_stub(180)
        interrupt_stub(181)
        interrupt_stub(182)
        interrupt_stub(183)
        interrupt_stub(184)
        interrupt_stub(185)
        interrupt_stub(186)
        interrupt_stub(187)
        interrupt_stub(188)
        interrupt_stub(189)
        interrupt_stub(190)
        interrupt_stub(191)
        interrupt_stub(192)
        interrupt_stub(193)
        interrupt_stub(194)
        interrupt_stub(195)
        interrupt_stub(196)
        interrupt_stub(197)
        interrupt_stub(198)
        interrupt_stub(199)
        interrupt_stub(200)
        interrupt_stub(201)
        interrupt_stub(202)
        interrupt_stub(203)
        interrupt_stub(204)
        interrupt_stub(205)
        interrupt_stub(206)
        interrupt_stub(207)
        interrupt_stub(208)
        interrupt_stub(209)
        interrupt_stub(210)
        interrupt_stub(211)
        interrupt_stub(212)
        interrupt_stub(213)
        interrupt_stub(214)
        interrupt_stub(215)
        interrupt_stub(216)
        interrupt_stub(217)
        interrupt_stub(218)
        interrupt_stub(219)
        interrupt_stub(220)
        interrupt_stub(221)
        interrupt_stub(222)
        interrupt_stub(223)
        interrupt_stub(224)
        interrupt_stub(225)
        interrupt_stub(226)
        interrupt_stub(227)
        interrupt_stub(228)
        interrupt_stub(229)
        interrupt_stub(230)
        interrupt_stub(231)
        interrupt_stub(232)
        interrupt_stub(233)
        interrupt_stub(234)
        interrupt_stub(235)
        interrupt_stub(236)
        interrupt_stub(237)
        interrupt_stub(238)
        interrupt_stub(239)
        interrupt_stub(240)
        interrupt_stub(241)
        interrupt_stub(242)
        interrupt_stub(243)
        interrupt_stub(244)
        interrupt_stub(245)
        interrupt_stub(246)
        interrupt_stub(247)
        interrupt_stub(248)
        interrupt_stub(249)
        interrupt_stub(250)
        interrupt_stub(251)
        interrupt_stub(252)
        interrupt_stub(253)
        interrupt_stub(254)
        interrupt_stub(255)

        .global interrupt_common_stub
        .type interrupt_common_stub, @function

        interrupt_common_stub:
                pushal
                pushl %gs
                pushl %fs
                pushl %es
                pushl %ds
                pushl %ss
                cld
                mov $GDT_KERNEL_DATA_OFFSET, %ax
                mov %ax, %ds
                mov %ax, %es
                mov %ax, %fs
                mov $GDT_KERNEL_PER_CPU_DATA_OFFSET, %ax
                mov %ax, %gs
                pushl %esp
                call interrupt_common_handler
                addl $4, %esp
                popl %ds
                popl %es
                popl %fs
                popl %gs
                addl $4, %esp

                # TODO: for now interrupts start and return to kernel so reload kernel gs segment.
                # When user mode will be implemented remember this and act accordingly! 

                mov $GDT_KERNEL_PER_CPU_DATA_OFFSET, %ax
                mov %ax, %gs
                popal
                addl $8, %esp
                iret 
        .size interrupt_common_stub, . - interrupt_common_stub