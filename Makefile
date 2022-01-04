ARCH?=i386

BINUTILS_VERSION?=2.37
GCC_VERSION?=11.2.0

TARGET=$(ARCH)-elf

DOWNLOAD_DIR:=$(PWD)/downloads
TOOLCHAIN_DIR:=$(PWD)/toolchain
BINUTILS_DIR:=$(DOWNLOAD_DIR)/binutils-$(BINUTILS_VERSION)
BINUTILS_BUILD_DIR:=$(BINUTILS_DIR)/build-binutils
GCC_DIR:=$(DOWNLOAD_DIR)/gcc-$(GCC_VERSION)
GCC_BUILD_DIR:=$(GCC_DIR)/build-gcc

.PHONY: toolchain binutils gcc binutils-download gcc-download clean-all clean-build clean-downloads

.DEFAULT_GOAL:=toolchain

toolchain: binutils gcc

binutils: binutils-download
	@cd $(BINUTILS_DIR) && mkdir -p build-binutils && cd $(BINUTILS_BUILD_DIR) && ../configure --target=$(TARGET) --prefix=$(TOOLCHAIN_DIR) --with-sysroot --disable-nls --disable-werror
	@cd $(BINUTILS_BUILD_DIR) && make
	@cd $(BINUTILS_BUILD_DIR) && make install

gcc: gcc-download
	@cd $(GCC_DIR) && mkdir -p build-gcc && cd $(GCC_BUILD_DIR) && ../configure --target=$(TARGET) --prefix=$(TOOLCHAIN_DIR) --disable-nls --enable-languages=c --without-headers
	@cd $(GCC_BUILD_DIR) && PATH=$(TOOLCHAIN_DIR)/bin:$(PATH) make all-gcc
	@cd $(GCC_BUILD_DIR) && PATH=$(TOOLCHAIN_DIR)/bin:$(PATH) make all-target-libgcc
	@cd $(GCC_BUILD_DIR) && PATH=$(TOOLCHAIN_DIR)/bin:$(PATH) make install-gcc
	@cd $(GCC_BUILD_DIR) && PATH=$(TOOLCHAIN_DIR)/bin:$(PATH) make install-target-libgcc

binutils-download: $(BINUTILS_DIR).tar.gz

gcc-download: $(GCC_DIR).tar.gz

$(PWD)/downloads/binutils-$(BINUTILS_VERSION).tar.gz:
	@mkdir -p $(DOWNLOAD_DIR)
	@cd $(DOWNLOAD_DIR) && wget https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VERSION).tar.gz && tar -xvf binutils-$(BINUTILS_VERSION).tar.gz

$(PWD)/downloads/gcc-$(GCC_VERSION).tar.gz:
	@mkdir -p $(DOWNLOAD_DIR)
	@cd $(DOWNLOAD_DIR) && wget https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.gz && tar -xvf gcc-$(GCC_VERSION).tar.gz

clean-downloads:
	@rm -rf $(DOWNLOAD_DIR)/*

clean-build:
	@rm -rf $(TOOLCHAIN_DIR)/*
	@rm -rf $(BINUTILS_BUILD_DIR)/*
	@rm -rf $(GCC_BUILD_DIR)/*

clean-all:
	rm -rf $(TOOLCHAIN_DIR)
	rm -rf $(DOWNLOAD_DIR)
