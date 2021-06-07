#Most of these flags can be overriden by setting the corresponding environment variables before running make in the top level directory
#TODO: in the future a configuration script should be provided to customize settings before building
ARCH?=i386
COMPILER_TARGET?=i686-elf
AR=$(COMPILER_TARGET)-ar
AS=$(COMPILER_TARGET)-as
CC=$(COMPILER_TARGET)-gcc
PROJECT_DIR?=/home/dimitri/Osdev/yauos
SOURCE_DIR?=$(PROJECT_DIR)/src
ISO_DIR?=$(PROJECT_DIR)/iso

#-MD tells the compiler to generate the .d files necessary for dependency tracking of #include directives in the source files
CFLAGS?=-O2 -g -MD
LDFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lgcc

ARCH_DIR:=$(SOURCE_DIR)/arch/$(ARCH)

#include additonal compilation flags from the the architecture specific folder
include $(ARCH_DIR)/include.mk
#include dependency tacking files, on first build they will not exist so the '-' before the include directive option tells make \
to not throw an error if it doesn't find them
-include $(OBJS:.o=.d)

#add architecture specific flags to existing compilation flags and architecture specific objects
CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
kernel/kernel.o \
kernel/printk/printk.o\
kernel/lib/string.o\

OBJS=\
$(KERNEL_OBJS) \

LINK_LIST=\
$(LDFLAGS) \
$(KERNEL_OBJS) \
$(LIBS) \

.PHONY: all iso qemu clean

all: yauos

yauos: $(OBJS) $(ARCH_DIR)/linker.ld
	$(CC) -T $(ARCH_DIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot2 yauos

%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(SOURCE_DIR)

%.o : %.S
	$(CC) -c $< -o $@ $(CFLAGS) -I$(SOURCE_DIR)

%.o : %.s
	$(CC) -c $< -o $@ $(CFLAGS) -I$(SOURCE_DIR)

iso: yauos
	mkdir -p $(PROJECT_DIR)/iso
	mkdir -p $(ISO_DIR)/boot
	mkdir -p $(ISO_DIR)/boot/grub
	cp yauos $(ISO_DIR)/boot
	@echo > $(ISO_DIR)/boot/grub/grub.cfg 'menuentry "yauos" { multiboot2 /boot/yauos }'
	grub-mkrescue -o yauos.iso $(ISO_DIR)

qemu: iso
	qemu-system-$(ARCH) -cdrom yauos.iso

clean:
	rm -f yauos
	rm -f $(OBJS)
	rm -f $(OBJS:.o=.d)
	rm -f yauos.iso
	rm -rf $(ISO_DIR)/*
