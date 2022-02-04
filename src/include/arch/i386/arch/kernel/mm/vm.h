#ifndef _VM_H
#define _VM_H

#include <stdint.h>
#include <arch/paging.h>
#include <arch/types.h>

#define RECURSIVE_DIRECTORY_START_REGION 0xFFC00000
#define PROT_KERN 0x0
#define PROT_USER 0x4
#define PROT_READ 0x0
#define PROT_READ_WRITE 0x2

extern void flush_tlb_single(virt_addr_t);
extern page_directory_t kernel_directory;

ssize_t map_page(phys_addr_t, virt_addr_t, uint16_t);
void unmap_page(virt_addr_t);

#endif /** _VM_H */