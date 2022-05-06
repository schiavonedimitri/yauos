#include <stddef.h>
#include <stdint.h>
#include <arch/cpu/cpu.h>
#include <kernel/printk.h>
#include <platform/lapic.h>

void lapic_write(uint32_t reg, uint32_t value) {
    uint32_t volatile *local_apic_p = (uint32_t volatile*) local_apic_address;
    *(uint32_t*) ((size_t) local_apic_p + reg) = value;
}

uint32_t lapic_read(uint32_t reg) {
    uint32_t volatile *local_apic_p = (uint32_t volatile*) local_apic_address;
    return *(uint32_t*) ((size_t) local_apic_p + reg);    
}

void lapic_init() {
    lapic_write(LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER, 0x1 << 8);
}

void lapic_send_ipi(uint8_t destination_local_apic_id, uint32_t icr_low) {
    lapic_write(LAPIC_INTERRUPT_COMMAND_REGISTER_1, destination_local_apic_id << 24);
    lapic_write(LAPIC_INTERRUPT_COMMAND_REGISTER_0, icr_low);    
}


void lapic_send_eoi() {

}