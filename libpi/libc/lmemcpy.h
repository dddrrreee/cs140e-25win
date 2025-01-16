#ifndef __LLMEMCPY_H__
#define __LLMEMCPY_H__
#include "pointer-T.h"

#if 0
#define aligned(ptr, n)  ((unsigned)ptr % n == 0)
#define aligned4(ptr)  aligned(ptr,4)
#define aligned8(ptr)  aligned(ptr,8)
#endif

static inline void 
lmemset(void *dst, uint32_t v, size_t nbytes) {
    assert(dst);
    assert(nbytes%4 == 0);
    assert(is_aligned4(dst));

    uint32_t *p = dst;
    uint32_t n = nbytes / 4;
    for(unsigned i = 0; i < n; i++)
        p[i] = v;
}

static inline void 
lmemcpy(void *dst, void *src, size_t nbytes) {
    assert(dst);
    assert(src);
    assert(nbytes%4 == 0);
    assert(is_aligned4(dst));

    uint32_t *to = dst;
    uint32_t *from = src;
    uint32_t n = nbytes / 4;

    for(unsigned i = 0; i < n; i++)
        to[i] = from[i];
}

static inline void 
llmemcpy(void *dst, void *src, size_t nbytes) {
    assert(dst);
    assert(src);
    assert(nbytes%8 == 0);
    assert(is_aligned8(dst));

    uint64_t *to = dst;
    uint64_t *from = src;
    uint32_t n = nbytes / 8;

    for(unsigned i = 0; i < n; i++)
        to[i] = from[i];
}
#endif
