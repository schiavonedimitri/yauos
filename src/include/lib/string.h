#ifndef STRING_H
#define STRING_H

#include <stddef.h>

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
int strcmp(const char*, const char*);
char* strcpy(char* __restrict, const char* __restrict);

#endif /** STRING_H */
