#ifndef PMM_H
#define PMM_H

#include <arch/types.h>
#include <kernel/bootinfo.h>

void pmm_init(bootinfo_t*);
phys_addr_t pmm_get_free_frame();
void pmm_free_frame(phys_addr_t);

#endif /** PMM_H */
