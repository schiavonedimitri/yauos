#ifndef MP_H
        #define MP_H

        #include <stdbool.h>
        #include <stdint.h>
        #include <arch/types.h>

        extern bool smp;
        extern size_t num_cpus;
        extern phys_addr_t local_apic_address;
        extern phys_addr_t io_apic_address;

        /*
        * Intel MP spec definitions.
        *
        * Places to look for the mp floating pointer structure according to the mp specification.
        */

        #define BDA_EBDA_BASE_ADDRESS 0x40E
        #define BDA_EBDA_BASE_ADDRESS_LEFT_SHIFT 4
        #define BDA_BASE_MEMORY_SIZE_ADDRESS 0x413
        #define BIOS_ROM_STARTING_ADDRESS 0xE0000
        #define BIOS_ROM_ENDING_ADDRESS 0xFFFFF

        #define MP_FLOATING_POINTER_STRUCTURE_SIGNATURE "_MP_"
        #define MP_FLOATING_POINTER_STRUCTURE_SIGNATURE_SIZE 4
        #define MP_CONFIGURATION_TABLE_SIGNATURE "PCMP"
        #define MP_CONFIGURATION_TABLE_SIGNATURE_SIZE 4

        typedef struct mp_configuration_table_header {
        char signature[4];
        uint16_t base_table_length;
        uint8_t specification_revision;
        uint8_t checksum;
        char oem_id[8];
        char product_id[12];
        phys_addr_t oem_table_pointer;
        uint16_t oem_table_size;
        uint16_t entry_count;
        phys_addr_t local_apic_address;
        uint16_t extended_table_length;
        uint8_t extended_table_checksum;
        uint8_t reserved;    
        } mp_configuration_table_header_t;

        typedef struct mp_configuration_table_processor_entry {
        uint8_t type;
        uint8_t local_apic_id;
        uint8_t local_apic_version;
        uint8_t cpu_flags;
        uint32_t cpu_signature;
        uint32_t feature_flags;
        uint64_t reserved;
        } mp_configuration_table_processor_entry_t;;

        /*
        * Processor entry definitions.
        */

        #define MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_TYPE 0x0
        #define MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_USABLE_SHIFT 0x0
        #define MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_USABLE 0x1
        #define MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_UNUSABLE 0x0
        #define MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_IS_BSP_SHIFT 0x1
        #define MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_IS_AP 0x0
        #define MP_CONFIGURATION_TABLE_PROCESSOR_ENTRY_CPU_FLAGS_PROCESSOR_IS_BSP 0x1
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_ON_CHIP_FPU_SHIFT 0x0
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_ON_CHIP_FPU_NOT_PRESENT 0x0
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_ON_CHIP_FPU_PRESENT 0x1
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_MCE_SHIFT 0x7
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_MCE_NOT_DEFINED 0x0
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_MCE_DEFINED 0x1
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_CX8_SHIFT 0x8
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_CX8_UNSUPPORTED 0x0
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_CX8_SUPPORTED 0x1
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_ON_CHIP_APIC_SHIFT 0x9
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_ON_CHIP_APIC_NOT_PRESENT 0x0
        #define MP_CONFIGURATION_TABLE_PROCESSOR_FEATURES_ON_CHIP_FPU_PRESENT 0x1

        /*
        * Bus entry definitions.
        */

        #define MP_CONFIGURATION_TABLE_BUS_ENTRY_TYPE 0x1

        typedef struct mp_configuration_table_bus_entry {
        uint8_t type;
        uint8_t bus_id;
        char bus_type_string[6];
        } mp_configuration_table_bus_entry_t;

        /*
        * I/O Apic entry definitions.
        */

        #define MP_CONFIGURATION_TABLE_IO_APIC_ENTRY_TYPE 0x2
        #define MP_CONFIGURATION_TABLE_IO_APIC_FLAGS_USABLE_SHIFT 0x0
        #define MP_CONFIGURATION_TABLE_IO_APIC_FLAGS_UNUSABLE 0x0
        #define MP_CONFIGURATION_TABLE_IO_APIC_FLAGS_USABLE 0x1

        typedef struct mp_configuration_table_io_apic_entry {
        uint8_t type;
        uint8_t io_apic_id;
        uint8_t io_apic_version;
        uint8_t io_apic_flags;
        phys_addr_t io_apic_address;
        } mp_configuration_table_io_apic_entry_t;

        /*
        * I/O apic interrupt entry definitions.
        */

        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY 0x3
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY_TYPE_INT 0x0
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY_TYPE_NMI 0x1
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY_TYPE_SMI 0x2
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_ENTRY_TYPE_EXTINT 0x3
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_POLARITY_MASK 0x3
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_POLARITY_CONFORMS_TO_BUS_SPECIFICATION 0x0
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_POLARITY_ACTIVE_HIGH 0x1
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_POLARITY_ACTIVE_LOW 0x3
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_SHIFT 0x2
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_MASK 0x3
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_CONFORMS_TO_BUS_SPECIFICATION 0x0
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_EDGE_TRIGGERED 0x1
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_LEVEL_TRIGGERED 0x3
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_PCI_SOURCE_MASK 0x3
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_PCI_SOURCE_INT_A 0x0
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_PCI_SOURCE_INT_B 0x1
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_PCI_SOURCE_INT_C 0x2
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_PCI_SOURCE_INT_D 0x3
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_PCI_SOURCE_DEVICE_NUMBER_SHIFT 0x2
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_PCI_SOURCE_DEVICE_NUMBER_MASK 0x1F
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_IO_APIC_INT_IN_0 0x0
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_IO_APIC_INT_IN_1 0x1
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_IO_APIC_INT_IN_2 0x2
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_IO_APIC_INT_IN_3 0x3
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_IO_APIC_INT_IN_4 0x4
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_IO_APIC_INT_IN_5 0x5
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_IO_APIC_INT_IN_6 0x6
        #define MP_CONFIGURATION_TABLE_IO_APIC_INTERRUPT_IO_APIC_INT_IN_7 0x7

        typedef struct mp_configuration_table_io_apic_interrupt_entry {
                uint8_t type;
                uint8_t interrupt_type;
                uint16_t interrupt_flags;
                uint8_t source_bus_id;
                uint8_t source_bus_irq;
                uint8_t destination_io_apic_id;
                uint8_t destination_io_apic_intin;
        } mp_configuration_table_io_apic_interrupt_entry_t;

        /*
        * Local apic interrupt entry definitions.
        */

        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY 0x4
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY_TYPE_INT 0x0
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY_TYPE_NMI 0x1
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY_TYPE_SMI 0x2
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_ENTRY_TYPE_EXTINT 0x3
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_POLARITY_MASK 0x3
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_POLARITY_CONFORMS_TO_BUS_SPECIFICATION 0x0
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_POLARITY_ACTIVE_HIGH 0x1
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_POLARITY_ACTIVE_LOW 0x3
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_SHIFT 0x2
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_MASK 0x3
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_CONFORMS_TO_BUS_SPECIFICATION 0x0
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_EDGE_TRIGGERED 0x1
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_FLAGS_TRIGGER_MODE_LEVEL_TRIGGERED 0x3
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_LINT_0 0x0
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_LINT_1 0x1
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_LINT_2 0x2
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_LINT_3 0x3
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_LINT_4 0x4
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_LINT_5 0x5
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_LINT_6 0x6
        #define MP_CONFIGURATION_TABLE_LOCAL_APIC_INTERRUPT_LINT_7 0x7


        typedef struct mp_configuration_table_local_apic_interrupt_entry {
                uint8_t type;
                uint8_t interrupt_type;
                uint8_t interrupt_flags;
                uint8_t source_bus_id;
                uint8_t source_bus_irq;
                uint8_t destination_local_apic_id;
                uint8_t destination_local_apic_lintin;
        } mp_configuration_table_local_apic_interrupt_entry_t;

        /*
        * MP floating pointer strucure definition.
        */

        #define MP_FLOATING_POINTER_STRUCTURE_SPEC_REVISION_1_1 0x1
        #define MP_FLOATING_POINTER_STRUCTURE_SPEC_REVISION_1_4 0x4
        #define MP_FLOATiNG_POINTER_STRUCTURE_MP_CONFIGURATION_TABLE_PRESENT 0x0
        #define MP_FLOATING_POINTER_STRUCTURE_IMCRP_SHIFT 0x7
        #define MP_FLOATING_POINTER_STRUCTURE_IMCR_NOT_PRESENT 0x0
        #define MP_FLOATING_POINTER_STRUCTURE_IMCR_PRESENT 0x1

        typedef struct mp_floating_pointer_structure {
                char signature[4];
                mp_configuration_table_header_t *mp_configuration_table_header_address;
                uint8_t length;
                uint8_t specification_revision;
                uint8_t checksum;
                uint8_t mp_feature_bytes[5];
        } mp_floating_pointer_structure_t;

        void smp_init(void);

#endif /** MP_H */