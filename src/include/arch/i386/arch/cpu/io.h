#ifndef IO_H
#define IO_H

#include <stdint.h>

inline static uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile ("inb %%dx, %%al":"=a"(ret):"d"(port));
	return ret;
}

inline static uint16_t inw(uint16_t port) {
	uint16_t ret;
	asm volatile ("inw %%dx, %%ax":"=a"(ret):"dN"(port));
	return ret;
}

inline static uint32_t inl(uint16_t port) {
	uint32_t ret;
	asm volatile ("inl %%dx, %%eax":"=a"(ret):"d"(port));
	return ret;
}

inline static void outb(uint16_t port, uint8_t value) {
	asm volatile ("outb %%al, %%dx"::"d"((port)),"a"((value)));
}

inline static void outw(uint16_t port, uint16_t value) {
	asm volatile ("outw %%ax, %%dx"::"d"(port),"a"(value));
}

inline static void outl(uint16_t port, uint32_t value) {
	asm volatile ("outl %%eax, %%dx"::"d"(port),"a"(value));
}

#endif /** IO_H */
