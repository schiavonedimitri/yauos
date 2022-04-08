#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/cpu/cpu.h>
#include <arch/cpu/mp.h>
#include <arch/kernel/mm/vm.h>
#include <arch/mmu.h>
#include <kernel/bootmem.h>
#include <kernel/printk.h>
#include <lib/string.h>

cpu_data_t *cpu_data;
size_t num_cpus = 0;
virt_addr_t local_apic_address = 0;
bool smp = 0;

static uint8_t checksum(void* addr, size_t len) {
    uint8_t checksum = 0;
    uint8_t *p = (uint8_t*) addr;
    for (size_t i = 0; i < len; i++) {
        checksum += p[i];
    }
    return checksum;
}

static mp_floating_pointer_structure_t* mp_floating_pointer_structure_search(void* addr, size_t length) {
    mp_floating_pointer_structure_t *ret = NULL;
    uint8_t *end_address = (uint8_t*) addr + length;
    for (uint8_t *mp = (uint8_t*) ALIGN(addr, 16); mp < end_address; mp += sizeof(mp_floating_pointer_structure_t)) {
        if (memcmp(mp, MP_FLOATING_POINTER_STRUCTURE_SIGNATURE, 4) == 0 && checksum((void*) mp, sizeof(mp_floating_pointer_structure_t)) == 0) { 
            ret = (mp_floating_pointer_structure_t*) mp;
        }
    }
    return ret;
}

static void init_cpu_data(uint8_t *entry, size_t num_entries) {
    uint8_t *saved_entry = entry;
    for (size_t i = 0; i < num_entries; i++) {
        switch(*entry) {
            case MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_TYPE:
                mp_configuration_table_processor_entry_t *processor_entry = (mp_configuration_table_processor_entry_t*) entry;
                if (processor_entry->cpu_flags & MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_USABLE) {
                    num_cpus++;
                }
                entry += sizeof(mp_configuration_table_processor_entry_t);
                break;
            case MP_CONFIGURATION_TABLE_BUS_ENTRY_TYPE:
                entry += sizeof(mp_configuration_table_bus_entry_t);
                break;
            case MP_CONFIGURATION_TABLE_IO_APIC_ENTRY_TYPE:
                mp_configuration_table_io_apic_entry_t *io_apic_entry = (mp_configuration_table_io_apic_entry_t*) entry;
                if (io_apic_entry->io_apic_flags & MP_CONFIGURATION_TABLE_IO_APIC_FLAGS_USABLE) {
                }
                entry += sizeof(mp_configuration_table_io_apic_entry_t);
                break;
            case  MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY:
                entry += sizeof(mp_configuration_table_io_apic_interrupt_entry_t);
                break;
            case  MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY:
                entry += sizeof(mp_configuration_table_local_apic_interrupt_entry_t);
                break;
        }
    }
    cpu_data = (cpu_data_t*) b_malloc(sizeof(cpu_data_t) * num_cpus);
    if (!cpu_data) {
        panic("[KERNEL]: Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
    }
    memset(cpu_data, 0x0, sizeof(cpu_data_t) * num_cpus);
    for (size_t i = 0; i < num_entries; i++) {
        switch(*saved_entry) {
            case MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_TYPE:
                mp_configuration_table_processor_entry_t *processor_entry = (mp_configuration_table_processor_entry_t*) saved_entry;
                if (processor_entry->cpu_flags & MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_USABLE) {
                    cpu_data->lapic_id = processor_entry->local_apic_id;
                    if ((processor_entry->cpu_flags >> MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_IS_BSP_SHIFT) & MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_IS_BSP) {
                        cpu_data->bsp = 1;
                    }
                    cpu_data++;
                }
                saved_entry += sizeof(mp_configuration_table_processor_entry_t);
                break;
            case MP_CONFIGURATION_TABLE_BUS_ENTRY_TYPE:
                saved_entry += sizeof(mp_configuration_table_bus_entry_t);
                break;
            case MP_CONFIGURATION_TABLE_IO_APIC_ENTRY_TYPE:
                mp_configuration_table_io_apic_entry_t *io_apic_entry = (mp_configuration_table_io_apic_entry_t*) saved_entry;
                if (io_apic_entry->io_apic_flags & MP_CONFIGURATION_TABLE_IO_APIC_FLAGS_USABLE) {
                }
                saved_entry += sizeof(mp_configuration_table_io_apic_entry_t);
                break;
            case  MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY:
                saved_entry += sizeof(mp_configuration_table_io_apic_interrupt_entry_t);
                break;
            case  MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY:
                saved_entry += sizeof(mp_configuration_table_local_apic_interrupt_entry_t);
                break;
        }
    }
    cpu_data -= num_cpus;
}

/*
 * As per Intel MP specification document search for an MP floating pointer structure at suggested locations:
 * A) In the first Kilobyte of the EBDA if defined or withing the last Kilobyte of the system base memory.
 * EBDA base address is usually specified in the BDA on x86 systems and systemd base memory size is also
 * specified in the BDA for x86 systems usually.
 * B) In the BIOS ROM space from 0xE0000 - 0xFFFFF.
 * The MP floating pointer structure must have "_MP_" as the signature and the checksum of all the fields
 * added with the checksum field must add up to 0 for the structure to be valid and thus for the system
 * to be MP compliant.
 * Note: all the locations must be aligned on a 16 byte boundary.
 */

void mp_init() {
    /* 
     * Not checking if the ebda is mapped in virtual memory because implicitly assuming it's in the low 1Mib memory
     * and the kernel boot code maps the first 2Mib atleast.
     */
    virt_addr_t ebda_address = PHYSICAL_TO_VIRTUAL(*(uint16_t*) PHYSICAL_TO_VIRTUAL(BDA_EBDA_BASE_ADDRESS) << BDA_EBDA_BASE_ADDRESS_LEFT_SHIFT);
    mp_floating_pointer_structure_t *mp = NULL;
    if (ebda_address) {
        mp = mp_floating_pointer_structure_search((void*) ebda_address, 0x400);
        if (mp == NULL) {
            mp = mp_floating_pointer_structure_search((void*) PHYSICAL_TO_VIRTUAL(BIOS_ROM_STARTING_ADDRESS), BIOS_ROM_ENDING_ADDRESS - BIOS_ROM_STARTING_ADDRESS);
            if (mp == NULL) {
                goto mp_not_compliant;
            }
            else {
                goto valid_floating_pointer_structure;
            }
        }
        else {
            goto valid_floating_pointer_structure;
        }
    }
    else {
        uint32_t base_memory_size = *((uint16_t*) PHYSICAL_TO_VIRTUAL(BDA_BASE_MEMORY_SIZE_ADDRESS)) - 0x400;
        mp = mp_floating_pointer_structure_search((void*) ((base_memory_size - 1) * 1024), 1024);
        if (mp == NULL) {
            goto mp_not_compliant;
        }
        else {
            goto valid_floating_pointer_structure;
        }
    }
    valid_floating_pointer_structure:
        if (mp->mp_feature_bytes[0] == MP_FLOATiNG_POINTER_STRUCTURE_MP_CONFIGURATION_TABLE_PRESENT) {
            mp_configuration_table_header_t *mp_config = (mp_configuration_table_header_t*) PHYSICAL_TO_VIRTUAL(mp->mp_configuration_table_header_address);
            if (memcmp(mp_config->signature, MP_CONFIGURATION_TABLE_SIGNATURE, MP_CONFIGURATION_TABLE_SIGNATURE_SIZE) == 0 && checksum((void*) mp_config, mp_config->base_table_length) == 0) {
                init_cpu_data((uint8_t*) (mp_config + 1), mp_config->entry_count);
                if (num_cpus > 0) {
                    // Identity mapping local apic address to the same virtual address.
                    map_page(mp_config->local_apic_address, mp_config->local_apic_address, PROT_PRESENT | PROT_READ_WRITE | PROT_KERN | PROT_CACHE_DISABLE);
                    local_apic_address = mp_config->local_apic_address;
                    smp = true;
                } 
            }
        }
        return;
    mp_not_compliant:
        return;
}