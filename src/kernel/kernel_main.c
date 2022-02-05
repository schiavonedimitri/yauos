#include <stdint.h>
#include <kernel/bootinfo.h>
#include <kernel/printk.h>
#include <kernel/mm/kmalloc.h>

void kernel_main(bootinfo_t *boot_info) {
	printk("[KERNEL]: Init done.\n");
	if (k_malloc_init() != -1) {
		uint32_t *array = NULL;
		for(size_t i = 0; i < 1024; i++) {
			array = (uint32_t*) k_malloc(sizeof(uint32_t) * 1024);
			if (array != NULL) {
				for (size_t j = 0; j < 1024; j++) {
					array[j] = j;	
				}
			}
			else {
				printk("Failed to allocate memory!\n");
			}
		}
		for (size_t i = 0; i < 1024; i++) {
			printk("array[%d] = %d\n", i, array[i]);
		}
		k_free(array);
		uint32_t * array2 = k_zmalloc(sizeof(uint32_t) * 1024);
		for (size_t i = 0; i < 1024; i++) {
			printk("array2[%d] = %d\n", i, array2[i]);
		}
	}
	else {
		panic("Failed to initialize kernel heap! File: %s line: %d function: %s\n", __FILENAME__, __LINE__, __func__);
	}
}