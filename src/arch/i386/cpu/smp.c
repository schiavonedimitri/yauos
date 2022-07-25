#include <cpuid.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch/align.h>
#include <arch/cpu/cpu.h>
#include <arch/cpu/smp.h>
#include <arch/kernel/mm/vm.h>
#include <arch/mmu.h>
#include <kernel/bootmem.h>
#include <kernel/printk.h>
#include <lib/string.h>

cpu_data_t *cpu_data;
size_t num_cpus = 0;
phys_addr_t local_apic_address = 0;
phys_addr_t io_apic_address = 0;
bool hyperthreading = false;
bool smp = false;

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

static void check_hyperthreading(void) {
    unsigned int eax, unused, edx = 0;
    __get_cpuid(1, &eax, &unused, &unused, &edx);
   hyperthreading = edx & (1 << 28);
   return;
}

static int init_cpu_data(uint8_t *entry, size_t num_entries) {
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
                    io_apic_address = io_apic_entry->io_apic_address;
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
    
    /*
     * In case the system is MP compliant but we find only 1 cpu
     */
    
    if (num_cpus == 1) {
        cpu_data = (cpu_data_t*) b_malloc(sizeof(cpu_data_t) * num_cpus);
        if (!cpu_data) {
            panic("[KERNEL]: Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
        }
        local_apic_address = 0;
        io_apic_address = 0;
        return -1;
    }
    /*
     * As read on the Intel developer's manual and on the Osdev forums https://forum.osdev.org/
     * if the cpu supports hyperthreading the total number of cpus on the systems is simply the
     * total count of physical cores multiplied by 2 (each core has 2 threads).
     */
    check_hyperthreading();
    if (hyperthreading) {
        num_cpus *= 2;
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
                    /*
                     * As read on the osdev forums, the local apic identifider of the hyperthreading cpus
                     * can be assumed to be the last physical core cpu local apic identifier increased by one.
                     * This was tested on a real machine and even though hacky it works.
                     */
                    if (hyperthreading) {
                        cpu_data->bsp = 0;
                        cpu_data->lapic_id = processor_entry->local_apic_id + 1;
                        cpu_data++;
                    }
                }
                saved_entry += sizeof(mp_configuration_table_processor_entry_t);
                break;
            case MP_CONFIGURATION_TABLE_BUS_ENTRY_TYPE:
                saved_entry += sizeof(mp_configuration_table_bus_entry_t);
                break;
            case MP_CONFIGURATION_TABLE_IO_APIC_ENTRY_TYPE:
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
    return 0;
}

#ifdef DEBUG

    /*
    * Useful for printing the mp table information.
    */

    static void print_table(uint8_t *entry, size_t num_entries) {
        uint32_t pci_bus_id = -1;
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
                    printk("Found bus entry:\nBus id: %x\nBus type: ", bus_entry->bus_id);
                    for (size_t i = 0; i < 6; i++) {
                        printk("%c", bus_entry->bus_type_string[i]);
                    }
                    printk("\n_____________\n");
                    if (memcmp(bus_entry->bus_type_string, "PCI", 3) == 0) {
                        pci_bus_id = bus_entry->bus_id;
                    }
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
                    if (io_apic_interrupt_entry->source_bus_id == pci_bus_id) {
                        char* int_to_s[4] = {"INT_A#", "INT_B#", "INT_C#", "INT_D#"};
                        uint8_t pci_int_n = io_apic_interrupt_entry->source_bus_irq & 0x3;
                        uint8_t pci_dev_n = (io_apic_interrupt_entry->source_bus_irq  >> 2) & 0x1F;
                        printk("PCI interrupt signal: %s\nPCI device number: %x\n_____________\n", int_to_s[pci_int_n], pci_dev_n);
                    }
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
#endif

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

void smp_init() {

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
                goto failure;
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
            goto failure;
        }
        else {
            goto valid_floating_pointer_structure;
        }
    }
    valid_floating_pointer_structure:
        if (mp->mp_feature_bytes[0] == MP_FLOATiNG_POINTER_STRUCTURE_MP_CONFIGURATION_TABLE_PRESENT) {
            mp_configuration_table_header_t *mp_config = (mp_configuration_table_header_t*) PHYSICAL_TO_VIRTUAL(mp->mp_configuration_table_header_address);
            if (memcmp(mp_config->signature, MP_CONFIGURATION_TABLE_SIGNATURE, MP_CONFIGURATION_TABLE_SIGNATURE_SIZE) == 0 && checksum((void*) mp_config, mp_config->base_table_length) == 0) {
                
                /*
                 * Handle the case where the system is MP compliant but has only one cpu.
                 */

                if (init_cpu_data((uint8_t*) (mp_config + 1), mp_config->entry_count)) {
                    goto failure;
                }
                else {
                    local_apic_address = mp_config->local_apic_address;
                    smp = true;
                    #ifdef DEBUG
                        print_table((uint8_t*) (mp_config + 1), mp_config->entry_count);
                    #endif
                    return;
                }
            }
        }

    /*
     * In case the system is not MP compliant allocate one cpu_data structure
     * because other kernel subsystems rely on it even on single core machines.
     */
    
    failure:
        cpu_data = (cpu_data_t*) b_malloc(sizeof(cpu_data_t));
        if (!cpu_data) {
            panic("[KERNEL]: Failed to allocate memory! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
        }
        cpu_data->bsp = true;
        return;
}