#include <stdint.h>
#include <kernel/bootinfo.h>
#include <kernel/pmm.h>
#include <kernel/printk.h>

extern uint32_t kernel_directory[1024];
extern uint32_t boot_page_table[1024];
extern void flush_tlb_single(virt_addr_t);

// Test mapping virtual addresses to physical frames.
void vm_alloc(virt_addr_t addr) {
	size_t dir_idx = (addr >> 22) & 0x3FF;
	size_t tbl_idx = (addr >> 12) & 0x3FF;
	if (!kernel_directory[dir_idx] & 0x1) {
		// Page table does not exist, allocate one.
		phys_addr_t new_table = pmm_get_free_frame();
		if (new_table == (phys_addr_t) -1) {
			panic("Failled to allocate memory for new_table!\n");
		}
		kernel_directory[dir_idx] = new_table | 0x3;
		/*
		* Since we have used a recursive directory mapping, the virtual address range 0xFFC00000 - 0xFFFFF000
		* are used to access page tables without the need to manually map them into the virtual address space. 
		*/
		uint32_t *table_virtual = (uint32_t*) (0xFFC00000 + (dir_idx * 0x1000));
		table_virtual[tbl_idx] = pmm_get_free_frame() | 0x3;
		flush_tlb_single(addr);
		return;
	}
	else {
		// Page table already exists and is the kernel boot page table.		
		boot_page_table[tbl_idx] = pmm_get_free_frame() | 0x3;
		flush_tlb_single(addr);
		return;
	}
}

void kmain(bootinfo_t *boot_info) {
	pmm_init(boot_info);
	printk("[KERNEL]: Init done.\n");
	// Testing mapping a virtual address to a random physical frame.
	// This is run before the memory allocation test because otherwise there would be no left physical frames for mapping.
	vm_alloc(0xC0F000);
	uint32_t *test = (uint32_t*) 0xC0F000;
	// Read test
	printk("Trying to read from %x\nRead value: %x\n", test, *test);
	// Write test
	printk("Trying to write to %x\nWriting 0xDEADBEEF\n");
	*test = 0xDEADBEEF;
	printk("Reading back from %x\nValue read: %x\n", test, *test);
	// Testing that physical frames are allocated only for available ranges.
	for (;;) {
		phys_addr_t frame = pmm_get_free_frame();
		// -1 is returned in a oom condition.
		if (frame != (phys_addr_t) -1) {
			// Fail if allocation happens in a known reserved region.
			if (frame == (phys_addr_t) 0x0 || frame == (phys_addr_t) 0x7000 || frame == (phys_addr_t) 0x80000 || frame == (phys_addr_t) 0xF00000) {
				panic("Allocated from reserved region!\n");
			}
		}
		else {
			// Break out of loop when memory is full.
			printk("Physical memory allocation test succeeded.\n");
			break;
		}
	}
}
