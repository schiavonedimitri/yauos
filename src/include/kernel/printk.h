#ifndef PRINTK_H
#define PRINTK_H
#include <stdbool.h>

void _printk(bool panic, const char* __restrict, ...);

#define printk(format, ...) _printk(0, format, ## __VA_ARGS__)
#define panic(format, ...) _printk(1, format, ## __VA_ARGS__)

#endif /** PRINTK_H */
