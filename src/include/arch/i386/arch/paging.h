#ifndef _PAGING_H
#define _PAGING_H

#include <stdint.h>

/*
 * This kernel only supports standard paging without page size extensions.
 */

struct page_directory_entry {
    uint8_t present: 1;
    uint8_t read_write: 1;
    uint8_t user_supervisor: 1;
    uint8_t page_write_through: 1;
    uint8_t page_cache_disable: 1;
    uint8_t accessed: 1;
    uint8_t unused_0 :1;
    uint8_t size: 1;
    uint8_t unused_1: 4;
    uint32_t address: 20;
} __attribute__((packed));

typedef struct page_directory_entry page_directory_entry_t;

typedef struct page_directory {
    page_directory_entry_t entry[1024];
} page_directory_t;

struct page_table_entry {
    uint8_t present: 1;
    uint8_t read_write: 1;
    uint8_t user_supervisor: 1;
    uint8_t page_write_through: 1;
    uint8_t page_cache_disable: 1;
    uint8_t accessed: 1;
    uint8_t dirty: 1;
    uint8_t page_attribute_table: 1;
    uint8_t global: 1;
    uint8_t unused: 3;
    uint32_t address: 20;
} __attribute__((packed));

typedef struct page_table_entry page_table_entry_t;

typedef struct page_table {
    page_table_entry_t entry[1024];
} page_table_t;

#endif /** _PAGING_H */
