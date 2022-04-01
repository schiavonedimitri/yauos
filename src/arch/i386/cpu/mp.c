#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/cpu/mp.h>
#include <arch/mmu.h>
#include <kernel/printk.h>
#include <lib/string.h>

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
        if (memcmp(mp, MP_FLOATING_POINTER_STRUCTURE_SIGNATURE, 4) == 0) { 
            if (checksum((void*) mp, sizeof(mp_floating_pointer_structure_t)) == 0) {
                ret = (mp_floating_pointer_structure_t*) mp;
            }
        }
    }
    return ret;
}

static void print_table(uint8_t *entry, size_t num_entries) {
    for (size_t i = 0; i < num_entries; i++) {
        switch(*entry) {
            case MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_TYPE:
                mp_configuration_table_processor_entry_t *processor_entry = (mp_configuration_table_processor_entry_t*) entry;
                if (processor_entry->cpu_flags & MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_USABLE) {
                    printk("Found processor entry:\nLocal apic id: %x\nLocal apic version: %x\n%s\n_____________\n", processor_entry->local_apic_id, processor_entry->local_apic_version, (processor_entry->cpu_flags >> MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_IS_BSP_SHIFT) & MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_IS_BSP ? "processor is BSP" : "processor is AP");
                }
                entry += sizeof(mp_configuration_table_processor_entry_t);
                break;
            case MP_CONFIGURATION_TABLE_BUS_ENTRY_TYPE:
                mp_configuration_table_bus_entry_t *bus_entry = (mp_configuration_table_bus_entry_t*) entry;
                printk("Found bus entry:\nBus id: %x\nBus type: %s\n_____________\n", bus_entry->bus_id, bus_entry->bus_type_string);
                entry += sizeof(mp_configuration_table_bus_entry_t);
                break;
            case MP_CONFIGURATION_TABLE_IO_APIC_ENTRY_TYPE:
                mp_configuration_table_io_apic_entry_t *io_apic_entry = (mp_configuration_table_io_apic_entry_t*) entry;
                if (io_apic_entry->io_apic_flags & MP_CONFIGURATION_TABLE_IO_APIC_FLAGS_USABLE) {
                    printk("Found I/O Apic entry:\nI/O Apic id: %x\nI/O Apic version: %x\nI/O Apic physical address: %x\n_____________\n", io_apic_entry->io_apic_id, io_apic_entry->io_apic_version, io_apic_entry->io_apic_address);
                }
                entry += sizeof(mp_configuration_table_io_apic_entry_t);
                break;
            case  MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY:
                mp_configuration_table_io_apic_interrupt_entry_t *io_apic_interrupt_entry = (mp_configuration_table_io_apic_interrupt_entry_t*) entry;
                printk("Found I/O Apic interrupt entry:\n");
                switch(io_apic_interrupt_entry->interrupt_type) {
                    case MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY_TYPE_INT:
                        printk("Interrupt type: INT (vectored interrupt)\n");
                        break;
                    case MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY_TYPE_NMI:
                        printk("Interrupt type: NMI\n");
                        break;
                    case MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY_TYPE_SMI:
                        printk("Interrupt type: SMI\n");
                        break;
                    case MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY_TYPE_EXTINT:
                        printk("Interrupt type: ExtINT\n");
                        break;
                }
                switch (io_apic_interrupt_entry->interrupt_flags & MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_POLARITY_MASK) {
                    case MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_POLARITY_CONFORMS_TO_BUS_SPECIFICATION:
                        printk("Polarity: conforms to bus specification\n");
                        break;
                    case MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_POLARITY_ACTIVE_LOW:
                        printk("Polarity: active low\n");
                        break;
                    case MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_POLARITY_ACTIVE_HIGH:
                        printk("Polarity: active high\n");
                        break;
                }
                switch ((io_apic_interrupt_entry->interrupt_flags >> MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_SHIFT) & MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_MASK) {
                    case MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_CONFORMS_TO_BUS_SPECIFICATION:
                        printk("Trigger mode: conforms to bus specification\n");
                        break;
                    case MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_EDGE_TRIGGERED:
                        printk("Trigger mode: edge triggered\n");
                        break;
                    case MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_LEVEL_TRIGGERED:
                         printk("Trigger mode: level triggered\n");
                         break;
                }
                printk("Source bus id: %x\nSource bus IRQ: %x\nDestination I/O apic id: %x\nDestination I/O apic INTIN#: %x\n_____________\n", io_apic_interrupt_entry->source_bus_id, io_apic_interrupt_entry->source_bus_irq, io_apic_interrupt_entry->destination_io_apic_id, io_apic_interrupt_entry->destination_io_apic_intin);
                entry += sizeof(mp_configuration_table_io_apic_interrupt_entry_t);
                break;
            case  MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY:
                mp_configuration_table_local_apic_interrupt_entry_t *local_apic_interrupt_entry = (mp_configuration_table_local_apic_interrupt_entry_t*) entry;
                printk("Found local apic interrupt entry:\n");
                switch(local_apic_interrupt_entry->interrupt_type) {
                    case MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY_TYPE_INT:
                        printk("Interrupt type: INT (vectored interrupt)\n");
                        break;
                    case MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY_TYPE_NMI:
                        printk("Interrupt type: NMI\n");
                        break;
                    case MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY_TYPE_SMI:
                         printk("Interrupt type: SMI\n");
                         break;
                    case MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY_TYPE_EXTINT:
                        printk("Interrupt type: ExtINT\n");
                        break;
                }
                switch (local_apic_interrupt_entry->interrupt_flags & MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_POLARITY_MASK) {
                    case MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_POLARITY_CONFORMS_TO_BUS_SPECIFICATION:
                        printk("Polarity: conforms to bus specification\n");
                        break;
                    case MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_POLARITY_ACTIVE_LOW:
                        printk("Polarity: active low\n");
                        break;
                    case MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_POLARITY_ACTIVE_HIGH:
                        printk("Polarity: active high\n");
                        break;
                }
                switch ((local_apic_interrupt_entry->interrupt_flags >> MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_SHIFT) & MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_MASK) {
                    case MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_CONFORMS_TO_BUS_SPECIFICATION:
                        printk("Trigger mode: conforms to bus specification\n");
                        break;
                    case MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_EDGE_TRIGGERED:
                        printk("Trigger mode: edge triggered\n");
                        break;
                    case MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_LEVEL_TRIGGERED:
                        printk("Trigger mode: level triggered\n");
                        break;
                }
                printk("Source bus id: %x\nSource bus IRQ:%x\nDestination local apic id: %x\nDestination local apic LINTIN#: %x\n_____________\n", local_apic_interrupt_entry->source_bus_id, local_apic_interrupt_entry->source_bus_irq, local_apic_interrupt_entry->destination_local_apic_id, local_apic_interrupt_entry->destination_local_apic_lintin);
                entry += sizeof(mp_configuration_table_local_apic_interrupt_entry_t);
                break;
        }
    }
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
        printk("MP floating pointer structure found at: %x\nMP specification revision: %d\nMP default configuration: %d\nMP configuration table physical address: %x\n", mp, mp->specification_revision, mp->mp_feature_bytes[0], mp->mp_configuration_table_header_address);
        if (mp->mp_feature_bytes[0] == 0) {
            mp_configuration_table_header_t *mp_config = (mp_configuration_table_header_t*) PHYSICAL_TO_VIRTUAL(mp->mp_configuration_table_header_address);
            if (memcmp(mp_config->signature, MP_CONFIGURATION_TABLE_SIGNATURE, MP_CONFIGURATION_TABLE_SIGNATURE_SIZE) == 0 && checksum((void*) mp_config, mp_config->base_table_length) == 0) {
                printk("MP configuration header table found:\nOem id: %s\nProduct id: %s\nLocal apic physical address: %x\n", mp_config->oem_id, mp_config->product_id, mp_config->local_apic_address);
                print_table((uint8_t*) (mp_config + 1), mp_config->entry_count);    
            }
            else {
                printk("Invalid configuration table found!\n");
            }
        }
        else {
            printk("MP default configurations not yet supported!");
        }
        return;
    mp_not_compliant:
        printk("System is not MP compliant!\n");
}