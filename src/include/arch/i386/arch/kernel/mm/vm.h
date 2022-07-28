#ifndef _VM_H
#define _VM_H

#include <stdbool.h>
#include <stdint.h>
#include <arch/paging.h>
#include <arch/types.h>

#define RECURSIVE_DIRECTORY_START_REGION 0xFFC00000
#define PROT_PRESENT 0x1
#define PROT_NOT_PRESENT 0x0
#define PROT_READ 0x0
#define PROT_READ_WRITE 0x2
#define PROT_USER 0x4
#define PROT_KERN 0x0
#define PROT_WRITE_THROUGH 0x8
#define PROT_WRITE_BACK 0x0
#define PROT_CACHE_ENABLE 0x0
#define PROT_CACHE_DISABLE 0x10
#define PROT_GLOBAL 0x100
#define PROT_NOT_GLOBAL 0x0

extern void flush_tlb_single(virt_addr_t);
extern page_directory_t kernel_directory;

int map_page(phys_addr_t, virt_addr_t, uint16_t, bool);
int unmap_page(virt_addr_t);
void do_page_fault(virt_addr_t);

#endif /** _VM_H */