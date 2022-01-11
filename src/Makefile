ARCH?=i386
PLATFORM?=pc

COMPILER_DIR:=$(realpath ..)/toolchain/bin
COMPILER_TARGET:=$(COMPILER_DIR)/$(ARCH)-elf

AR:=$(COMPILER_TARGET)-ar
AS:=$(COMPILER_TARGET)-as
CC:=$(COMPILER_TARGET)-gcc
LD:=$(COMPILER_TARGET)-ld
OBJDUMP:=$(COMPILER_TARGET)-objdump
OBJCOPY:=$(COMPILER_TARGET)-objcopy
READELF:=$(COMPILER_TARGET)-readelf

PROJECT_DIR:=$(realpath ..)
SRC_DIR:=$(PROJECT_DIR)/src
ARCH_DIR:=$(SRC_DIR)/arch/$(ARCH)
PLATFORM_DIR:=$(SRC_DIR)/arch/$(ARCH)/platform/$(PLATFORM)
INCLUDE_DIR:=$(SRC_DIR)/include
INCLUDE_ARCH_DIR:=$(INCLUDE_DIR)/arch/$(ARCH)
INCLUDE_PLATFORM_DIR:=$(INCLUDE_ARCH_DIR)/platform/$(PLATFORM)
LIB_DIR=$(SRC_DIR)/lib
ISO_DIR	:=$(PROJECT_DIR)/iso
OBJ_DIR:=$(PROJECT_DIR)/obj
BIN_DIR:=$(PROJECT_DIR)/bin

BOOTCONSOLE?=VGA
EARLY_HEAP_SIZE?=4096
DEBUG_ENABLE?=0

ifeq ($(DEBUG_ENABLE), 1)
	DEBUG:=-DDEBUG
	DEBUG_INFO:=-g
else
	DEBUG:=
	DEBUG_INFO:=
endif

ifeq ($(ARCH), i386)
	ARCH_BITS:=32
endif

ifeq ($(ARCH), x64)
	ARCH_BITS=64
endif

CFLAGS:=-O3 $(DEBUG_INFO) -MMD -MP -I$(INCLUDE_DIR) -I$(INCLUDE_ARCH_DIR) -I$(INCLUDE_PLATFORM_DIR)
CFLAGS+=-ffreestanding -Wall -Wextra -std=gnu11 -DARCH=$(ARCH) -DARCH_BITS=$(ARCH_BITS) -DEARLY_HEAP_SIZE=$(EARLY_HEAP_SIZE) -DBOOTCONSOLE=$(BOOTCONSOLE) $(DEBUG)
LDFLAGS:=-T $(ARCH_DIR)/$(ARCH).ld -nostdlib

LIBS:=-lgcc

QEMU_MEM?=128M

CFILES:=$(shell find $(SRC_DIR)  -not \( -path $(SRC_DIR)/arch -prune \) -name '*.c')
CFILES+=$(shell find $(ARCH_DIR) -not \( -path $(ARCH_DIR)/platform -prune \) -name '*.c')
CFILES+=$(shell find $(PLATFORM_DIR) -name '*.c')

ASFILES:=$(shell find $(SRC_DIR) -not \( -path $(SRC_DIR)/arch -prune \) -name '*.S')
ASFILES+=$(shell find $(ARCH_DIR) -not \( -path $(ARCH_DIR)/platform -prune \) -name '*.S')
ASFILES+=$(shell find $(PLATFORM_DIR) -name '*.S')

OBJS:=$(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(CFILES)))
OBJS+=$(patsubst %.S, $(OBJ_DIR)/%.o, $(notdir $(ASFILES)))

VPATH=$(sort $(dir $(CFILES)) $(dir $(ASFILES)))

LINK_LIST:=\
$(OBJS)\
$(LIBS)

-include $(OBJS:.o=.d)

.PHONY: all qemu qemu-gdb clean yauos iso

.DEFAULT_GOAL:=yauos

yauos: $(BIN_DIR)/yauos

iso: $(ISO_DIR)/yauos.iso

$(BIN_DIR)/yauos: $(OBJS) $(ARCH_DIR)/$(ARCH).ld
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(LINK_LIST)
	@grub-file --is-x86-multiboot2 $(BIN_DIR)/yauos

$(OBJ_DIR)/%.o : %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)/%.o : %.S
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(ISO_DIR)/yauos.iso: $(BIN_DIR)/yauos
	@mkdir -p $(ISO_DIR)
	@mkdir -p $(PROJECT_DIR)/tmp
	@mkdir -p $(PROJECT_DIR)/tmp/boot
	@mkdir -p $(PROJECT_DIR)/tmp/boot/grub
	@cp $(BIN_DIR)/yauos $(PROJECT_DIR)/tmp/boot
	@echo > $(PROJECT_DIR)/tmp/boot/grub/grub.cfg 'menuentry "yauos" { multiboot2 /boot/yauos bootconsole=serial }'
	@grub-mkrescue -o $(ISO_DIR)/yauos.iso $(PROJECT_DIR)/tmp
	@rm -rf $(PROJECT_DIR)/tmp

qemu: $(ISO_DIR)/yauos.iso
	@qemu-system-$(ARCH) -serial mon:stdio -nographic -m $(QEMU_MEM) -cdrom $(ISO_DIR)/yauos.iso

qemu-gdb: $(ISO_DIR)/yauos.iso
	@qemu-system-$(ARCH) -serial mon:stdio -nographic -s -S -m $(QEMU_MEM) -cdrom $(ISO_DIR)/yauos.iso

clean:
	@rm -rf $(BIN_DIR)/*
	@rm -rf $(OBJ_DIR)/*
	@rm -rf $(ISO_DIR)/*
