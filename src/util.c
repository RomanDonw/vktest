#include "util.h"

#include <stdlib.h>
#include <stdio.h>

static const char *errstr = "memory allocation failed";

void *malloc_s(size_t size)
{
    void *ret = malloc(size);
    if (!ret) { puts(errstr); abort(); }
    return ret;
}

void *realloc_s(void *block, size_t size)
{
    void *ret = realloc(block, size);
    if (!ret) { puts(errstr); abort(); }
    return ret;
}