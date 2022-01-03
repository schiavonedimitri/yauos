#ifndef BITMAP_H
#define BITMAP_H

#include <stddef.h>

void bitmap_set(void *bitmap, int bit);
void bitmap_unset(void *bitmap, int bit);
int bitmap_test(void *bitmap, int bit);
int bitmap_first_unset(void *bitmap, size_t size);

#endif /** BITMAP_H */
