#include <stdint.h>
#include <kernel/bootinfo.h>
#include <kernel/pmm.h>
#include <kernel/printk.h>
#include <arch/kernel/vmm.h>
#include <arch/mmu.h>

extern page_directory_t kernel_directory;
extern page_table_t boot_page_table;
extern flush_tlb_single(virt_addr_t);

void kmain(bootinfo_t *boot_info) {
	printk("[KERNEL]: Init done.\n");
}
