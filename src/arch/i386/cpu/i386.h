#ifndef i386_H
#define i386_H

#define ALIGN(address, bytes) (void*) (((uint32_t) (address) + bytes - 1) & ~(bytes - 1))

#endif
