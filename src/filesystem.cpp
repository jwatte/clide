#include "filesystem.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <assert.h>


char *fs_concat(char *dst, size_t dstsize, char const *a, char const *b)
{
    assert(dstsize > 0);
    *dst = 0;
    size_t sz = strlen(a);
    size_t bsz = strlen(b);
    if (sz + bsz + 2 > dstsize) {
        return NULL;
    }
    memcpy(dst, a, sz);
    if (bsz > 0) {
        dst[sz] = '/';
        memcpy(dst+sz+1, b, bsz);
        dst[sz+bsz+2] = 0;
    }
    return dst;
}


