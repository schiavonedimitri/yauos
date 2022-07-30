#include <stddef.h>
#include <stdint.h>
#include <arch/cpu/cpu.h>
#include <arch/cpu/lapic.h>
#include <arch/cpu/smp.h>
#include <kernel/printk.h>

void lapic_write(uint32_t reg, uint32_t value) {
        uint32_t volatile *local_apic_p = (uint32_t volatile*) local_apic_address;
        *(uint32_t*) ((size_t) local_apic_p + reg) = value;
}

uint32_t lapic_read(uint32_t reg) {
        uint32_t volatile *local_apic_p = (uint32_t volatile*) local_apic_address;
        return *(uint32_t*) ((size_t) local_apic_p + reg);    
}

/*
 * The 8th bit in the lapic spurious interrupt vector register enables the lapic. The value 0xFF was chosen
 * because it has the 8th bit set and is out of range of cpu exception numbers and out of range of the remapped
 * PIC range (32 - 47) see pic.c. The pic irq numbers are avoided even though the PIC is not used in a SMP system
 * because although the irqs are masked off they could still cause spurious irqs.
 */

void lapic_init() {
        lapic_write(LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER, 0x1FF);
}

void lapic_send_ipi(uint8_t destination_local_apic_id, uint32_t icr_low) {
        lapic_write(LAPIC_INTERRUPT_COMMAND_REGISTER_1, destination_local_apic_id << 24);
        lapic_write(LAPIC_INTERRUPT_COMMAND_REGISTER_0, icr_low);    
}


void lapic_send_eoi() {
        
}