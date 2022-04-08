#include <stdint.h>
#include <arch/mmu.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>

extern uintptr_t _AP_TEXT_START_;
extern uintptr_t _AP_TEXT_END_;

void kernel_main(bootinfo_t *boot_info) {
	printk("[KERNEL]: Arch init complete.\n");
	k_malloc_init();
	//printk("ap start address: %x\nap end address: %x\n", &_AP_TEXT_START_, &_AP_TEXT_END_);
}