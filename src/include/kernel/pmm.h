#ifndef PMM_H
#define PMM_H

#include <arch/types.h>
#include <kernel/bootinfo.h>

phys_addr_t get_free_frame();
void free_frame(phys_addr_t);

#endif /** PMM_H */
