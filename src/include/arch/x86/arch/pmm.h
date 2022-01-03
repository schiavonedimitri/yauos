#ifndef PMM_H
#define PMM_H

#include <arch/multiboot2.h>
#include <arch/types.h>

void pmm_init(multiboot2_information_header_t *m_boot2_info);
phys_addr_t pmm_get_free_frame();
void pmm_free_frame(phys_addr_t addr);


#endif /** PMM_H */
