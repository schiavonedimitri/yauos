#ifndef BITMAP_H
#define BITMAP_H

#include <stddef.h>

void bitmap_set(void*, int);
void bitmap_unset(void*, int);
int bitmap_test(void*, int);
int bitmap_first_unset(void*, size_t);

#endif /** BITMAP_H */
