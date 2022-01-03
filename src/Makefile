#Toolchain architecture options:
ARCH	:=	x86
COMPILER_TARGET	:=	i686-elf
AR	:=	$(COMPILER_TARGET)-ar
AS	:=	$(COMPILER_TARGET)-as
CC	:=	$(COMPILER_TARGET)-gcc
#Project directory options
PROJECT_DIR	:=	$(HOME)/Osdev/yauos
SOURCE_DIR	:=	$(PROJECT_DIR)/src
ARCH_DIR	:=	$(SOURCE_DIR)/arch/$(ARCH)
LIB_DIR	:=	$(SOURCE_DIR)/lib
ISO_DIR	:=$(PROJECT_DIR)/iso
#Size in bytes of the early boot heap
EARLY_HEAP_SIZE	?=	4096
DEBUG_ENABLE	?=	1
ifeq ($(DEBUG_ENABLE), 1)
DEBUG	:=	-DDEBUG
else
DEBUG	:=
endif
#Toolchain general flags
CFLAGS	:=	-O3 -MD -I$(SOURCE_DIR)/include -I$(SOURCE_DIR)/include/arch/$(ARCH)
CFLAGS	+=	-ffreestanding -Wall -Wextra -std=gnu11 -DARCH=$(ARCH) -DEARLY_HEAP_SIZE=$(EARLY_HEAP_SIZE) $(DEBUG)
LDFLAGS	:=	-T $(ARCH_DIR)/linker.ld -nostdlib
LIBS	:=	-lgcc
#Qemu settings
QEMU_MEM	:=	128M
QEMU_ARCH	:=	i386
#Objects to build
OBJS	:=	\
	$(ARCH_DIR)/halt/halt.o	\
	$(ARCH_DIR)/bootmem/bootmem.o	\
	$(ARCH_DIR)/pmm/pmm.o	\
	$(ARCH_DIR)/vmm/flush_tlb_all.o	\
	$(ARCH_DIR)/vmm/flush_tlb_single.o	\
	$(ARCH_DIR)/boot/boot.o	\
	$(ARCH_DIR)/gdt/gdt.o	\
	$(ARCH_DIR)/gdt/load_gdt.o	\
	$(ARCH_DIR)/vga/vga.o	\
	$(LIB_DIR)/string/string.o	\
	$(LIB_DIR)/bitmap/bitmap.o	\
	kernel/init/kernel.o	\
	kernel/printk/printk.o

LINK_LIST	:=	\
	$(OBJS)	\
	$(LIBS)

#Include gcc dependency files generated with -MD CFLAGS option for header files dependency tracking
-include $(OBJS:.o=.d)

.DEFAULT_GOAL	:=	yauos

.PHONY:	all qemu clean

yauos: $(OBJS) $(ARCH_DIR)/linker.ld
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot2 yauos

%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.o : %.s
	$(CC) -c $< -o $@ $(CFLAGS)

%.o : %.S
	$(CC) -c $< -o $@ $(CFLAGS)

yauos.iso: yauos
	mkdir -p $(PROJECT_DIR)/iso
	mkdir -p $(ISO_DIR)/boot
	mkdir -p $(ISO_DIR)/boot/grub
	cp yauos $(ISO_DIR)/boot
	@echo > $(ISO_DIR)/boot/grub/grub.cfg 'menuentry "yauos" { multiboot2 /boot/yauos }'
	grub-mkrescue -o yauos.iso $(ISO_DIR)

qemu: yauos.iso
	qemu-system-$(QEMU_ARCH) -monitor stdio -m $(QEMU_MEM) -cdrom yauos.iso

qemu-gdb: yauos.iso
	qemu-system-$(QEMU_ARCH) -monitor stdio -s -S -m $(QEMU_MEM) -cdrom yauos.iso

clean:
	rm -f yauos
	rm -f $(OBJS)
	rm -f $(OBJS:.o=.d)
	rm -f yauos.iso
	rm -rf $(ISO_DIR)
