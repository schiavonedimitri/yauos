#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

#include <stdint.h>

#define MULTIBOOT2_MAGIC 0x36d76289
#define MULTIBOOT2_TAG_ALIGN 8
#define MULTIBOOT2_TAG_END_TYPE 0
#define MULTIBOOT2_TAG_BOOT_COMMANDLINE_TYPE 1
#define MULTIBOOT2_TAG_BOOTLOADER_NAME_TYPE 2
#define MULTIBOOT2_TAG_MODULES_TYPE 3
#define MULTIBOOT2_TAG_BASIC_MEMORY_INFORMATION_TYPE 4
#define MULTIBOOT2_TAG_BIOS_BOOT_DEVICE_TYPE 5
#define MULTIBOOT2_TAG_MEMORY_MAP_TYPE 6
#define MULTIBOOT2_TAG_VBE_INFO_TYPE 7
#define MULTIBOOT2_TAG_FRAMEBUFFER_INFO_TYPE 8
#define MULTIBOOT2_TAG_ELF_SYMBOLS_TYPE 9
#define MULTIBOOT2_TAG_APM_TABLE_TYPE 10
#define MULTIBOOT2_TAG_EFI_32_SYSTEM_TABLE_POINTER_TYPE 11
#define MULTIBOOT2_TAG_EFI_64_SYSTEM_TABLE_POINTER_TYPE 12
#define MULTIBOOT2_TAG_SMBIOS_TABLES_TYPE 13
#define MULTIBOOT2_TAG_ACPI_OLD_RSDP_TYPE 14
#define MULTIBOOT2_TAG_ACPI_NEW_RSDP_TYPE 15
#define MULTIBOOT2_TAG_NETWORKING_INFORMATION_TYPE 16
#define MULTIBOOT2_TAG_EFI_MEMORY_MAP_TYPE 17
#define MULTIBOOT2_TAG_EFI_BOOT_SERVICES_NOT_TERMINATED_TYPE 18
#define MULTIBOOT2_TAG_EFI_32_BIT_IMAGE_HANDLE_POINTER_TYPE 19
#define MULTIBOOT2_TAG_EFI_64_BIT_IMAGE_HANDLE_POINTER_TYPE 20
#define MULTIBOOT2_TAG_IMAGE_LOAD_BASE_PHYSICAL_ADDRESS_TYPE 21

#define MULTIBOOT2_MEMORY_AVAILABLE 1
#define MULTIBOOT2_MEMORY_RESERVED 2
#define MULTIBOOT2_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT2_MEMORY_NVS 4
#define MULTIBOOT2_MEMORY_BADRAM 5

typedef struct multiboot2_information_header {
	uint32_t total_size;
	uint32_t reserved;
} multiboot2_information_header_t;

typedef struct multiboot2_tag_header {
	uint32_t type;
	uint32_t size;
} multiboot2_tag_header_t;

typedef struct multiboot2_tag_basic_memory_information {
	uint32_t type;
	uint32_t size;
	uint32_t mem_lower;
	uint32_t mem_upper;
} multiboot2_tag_basic_memory_information_t;

typedef struct multiboot2_tag_bios_boot_device {
	uint32_t type;
	uint32_t size;
	uint32_t biosdev;
	uint32_t partiton;
	uint32_t sub_partition;
} multiboot2_tag_bios_boot_device_t;

typedef struct multiboot2_tag_boot_command_line {
	uint32_t type;
	uint32_t size;
	uint8_t string[];
} multiboot2_tag_boot_command_line_t;

typedef struct multiboot2_tag_modules {
	uint32_t type;
	uint32_t size;
	uint32_t mod_start;
	uint32_t mod_end;
	uint8_t string[];
} multiboot2_tag_modules_t;

typedef struct multiboot2_tag_elf_symbols {
	uint32_t type;
	uint32_t size;
	uint16_t num;
	uint16_t entsize;
	uint16_t shndx;
	uint16_t reserved;
	uint8_t sections[];
} multiboot2_tag_elf_symbols_t;

typedef struct multiboot2_tag_memory_map_entry {
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;
} multiboot2_tag_memory_map_entry_t;

typedef struct multiboot2_tag_memory_map {
	uint32_t type;
	uint32_t size;
	uint32_t entry_size;
	uint32_t entry_version;
	multiboot2_tag_memory_map_entry_t entries[];
} multiboot2_tag_memory_map_t;

typedef struct multiboot2_tag_bootloader_name {
	uint32_t type;
	uint32_t size;
	uint8_t string[];
} multiboot2_tag_bootloader_name_t;

typedef struct multiboot2_tag_apm_table {
	uint32_t type;
	uint32_t size;
	uint16_t version;
	uint16_t cseg;
	uint32_t offset;
	uint16_t cseg_16;
	uint16_t dseg;
	uint16_t flags;
	uint16_t cseg_len;
	uint16_t cseg_16_len;
	uint16_t dseg_len;
} multiboot2_tag_apm_table_t;

typedef struct multiboot2_tag_vbe_info {
	uint32_t type;
	uint32_t size;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;
	uint8_t vbe_control_info[512];
	uint8_t vbe_mode_info[256];
} multiboot2_tag_vbe_info_t;

typedef struct multiboot2_tag_framebuffer_info_color_type {
	uint8_t framebuffer_red_field_position;
	uint8_t framebuffer_red_mask_size;
	uint8_t framebuffer_green_field_position;
	uint8_t framebuffer_green_mask_size;
	uint8_t framebuffer_blue_field_position;
	uint8_t framebuffer_blue_mask_size;
} multiboot2_tag_framebuffer_info_color_type_t;

typedef struct multiboot2_tag_framebuffer_info_color_info_framebuffer_palette {
	uint8_t red_value;
	uint8_t green_value;
	uint8_t blue_value;
} multiboot2_tag_framebuffer_info_color_info_framebuffer_palette_t;

typedef struct multiboot2_tag_framebuffer_info_color_info{
	uint32_t framebuffer_palette_num_colors;
	multiboot2_tag_framebuffer_info_color_info_framebuffer_palette_t framebuffer_palette[];
} multiboot2_tag_framebuffer_info_color_info_t;

typedef struct multiboot2_tag_framebuffer_info {
	uint64_t framebuffer_addr;
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint8_t framebuffer_bpp;
	uint8_t framebuffer_type;
	uint8_t reserved;
    union {
		multiboot2_tag_framebuffer_info_color_info_t color_info;
		multiboot2_tag_framebuffer_info_color_type_t color_type;
    };
} multiboot2_tag_framebuffer_info_t;

typedef struct multiboot2_tag_efi_32_system_table_pointer {
	uint32_t type;
	uint32_t size;
	uint32_t pointer;
} multiboot2_tag_efi_32_system_table_pointer_t;

typedef struct multiboot2_tag_efi_64_system_table_pointer {
	uint32_t type;
	uint32_t size;
	uint64_t pointer;
} multiboot2_tag_efi64_system_table_pointer_t;

typedef struct multiboot2_tag_smbios_tables {
	uint32_t type;
	uint32_t size;
	uint8_t major;
	uint8_t minor;
	uint8_t reserved[6];
	uint8_t tables[];
} multiboot2_tag_smbios_tables_t;

typedef struct multiboot2_tag_acpi_old_rsdp {
	uint32_t type;
	uint32_t size;
	uint8_t rsdp[];
} multiboot2_tag_acpi_old_rsdp_t;

typedef struct multiboot2_tag_acpi_new_rsdp {
	uint32_t type;
	uint32_t size;
	uint8_t rsdp[];
} multiboot2_tag_acpi_new_rsdp_t;

typedef struct multiboot2_tag_networking_information {
	uint32_t type;
	uint32_t size;
	uint8_t dhcpack[];
} multiboot2_tag_networking_information_t;

typedef struct multiboot2_tag_efi_memory_map {
	uint32_t type;
	uint32_t size;
	uint32_t descriptor_size;
	uint32_t descriptor_version;
	uint8_t efi_memory_map[];
} multiboot2_tag_efi_memory_map_t;

typedef struct multiboot2_tag_efi_boot_services_not_terminated {
	uint32_t type;
	uint32_t size;
} multiboot2_tag_efi_boot_services_not_terminated_t;

typedef struct multiboot2_tag_efi_32_bit_image_handle_pointer {
	uint32_t type;
	uint32_t size;
	uint32_t pointer;
} multiboot2_tag_efi_32_bit_image_handle_pointer_t;

typedef struct multiboot2_tag_efi_64_bit_image_handle_pointer {
	uint32_t type;
	uint32_t size;
	uint64_t pointer;
} multiboot2_tag_efi_64_bit_image_handle_pointer_t;

typedef struct multiboot2_tag_image_load_base_physical_address {
	uint32_t type;
	uint32_t size;
	uint32_t load_base_addr;
} multiboot2_tag_image_load_base_physical_address_t;

#endif /** MULTIBOOT2_H */
