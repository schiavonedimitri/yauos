#ifndef IO_H
        #define IO_H

        #include <stdint.h>

        static inline uint8_t inb(uint16_t port) {
                uint8_t ret;
                asm volatile ("inb %%dx, %%al":"=a"(ret):"d"(port));
                return ret;
        }

        static inline uint16_t inw(uint16_t port) {
                uint16_t ret;
                asm volatile ("inw %%dx, %%ax":"=a"(ret):"dN"(port));
                return ret;
        }

        static inline uint32_t inl(uint16_t port) {
                uint32_t ret;
                asm volatile ("inl %%dx, %%eax":"=a"(ret):"d"(port));
                return ret;
        }

        static inline void outb(uint16_t port, uint8_t value) {
                asm volatile ("outb %%al, %%dx"::"d"((port)),"a"((value)));
        }

        static inline void outw(uint16_t port, uint16_t value) {
                asm volatile ("outw %%ax, %%dx"::"d"(port),"a"(value));
        }

        static inline void outl(uint16_t port, uint32_t value) {
                asm volatile ("outl %%eax, %%dx"::"d"(port),"a"(value));
        }

        static inline void io_wait(void) {
                outb(0x80, 0);
        }

#endif /** IO_H */