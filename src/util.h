#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void *malloc_s(size_t size);
void *realloc_s(void *block, size_t size);

#endif