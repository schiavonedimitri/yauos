#ifndef _VM_H
#define _VM_H

#define RECURSIVE_DIRECTORY_START_REGION 0xFFC00000

extern page_directory_t kernel_directory;
extern page_table_t kernel_boot_page_table;


#endif /** _VM_H */