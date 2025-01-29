// engler: simple 140e circular buffer implementation.  should
// be thread safe for interrupt handler producer / non-interrupt
// consumer.

#ifndef __CIRCULAR_T_H__
#define __CIRCULAR_T_H__

// simple lock-free circular queue.
#ifndef RPI_UNIX
#   include "rpi.h"
// #   include "rpi-interrupts.h"
#else
// #   define printk printf
#   include <assert.h>
#   include <stdio.h>
#   include <stdlib.h>
#   include <string.h>
#   define int_is_enabled() 0
#endif

// single mutator of head, single mutator of tail = lock free.
// should work as a lock-free datastructure.
//
//  - pfx: prepend to all helper routines.
//  - CQ_T name of the circular queue type.
//  - E_T is the name of the element type.
//  - N is the static size of the circular queue.
#define gen_circular_T(pfx, CQ_T, E_T, N)           \
    typedef struct {                                            \
        E_T c_buf[N];                                           \
        unsigned fence;                                         \
        unsigned head, tail;                                    \
                                                                \
        /* number of times a push failed b/c full. */           \
        unsigned overflow;                                      \
        /* =1 -> we panic on error. */                          \
        unsigned errors_fatal_p:1;                              \
    } CQ_T;                                                \
                                                                \
    static inline int pfx ## _empty(CQ_T *q) {          \
        gcc_mb();                                               \
        return q->head == q->tail;                              \
    }                                                           \
    static inline int pfx ## _full(CQ_T *q) {           \
        gcc_mb();                                               \
        return (q->head+1) % (N) == q->tail;                    \
    }                                                           \
                                                                \
    static inline unsigned pfx ## _cnt(CQ_T *q) {     \
        gcc_mb();                                               \
        return (q->head - q->tail) % (N);                       \
    }                                                           \
                                                                \
    /* not blocking: requires interrupts. */                    \
    static inline int pfx ## _pop_nonblk(CQ_T *c, E_T *e) {  \
        if(pfx ## _empty(c))                                 \
            return 0;                                           \
                                                                \
        /* there is a memory barrier in cq_eq */                \
        unsigned tail = c->tail;                                \
        /* must occur in order. */                              \
        *e = c->c_buf[tail];     /* 1 */                        \
        gcc_mb();                                               \
        c->tail = (tail+1)% (N);  /* 2 */                       \
        gcc_mb();                                               \
        return 1;                                               \
    }                                                           \
                                                                \
    /* blocking: called from non-interrupt code. */             \
    static inline E_T pfx ## _pop(CQ_T *c) {                 \
        E_T e;                                                    \
        memset(&e, 0, sizeof e);                                \
                                                                \
	    /* FIXME: if used w/ interrupts need yield. */          \
        if(!pfx ## _pop_nonblk(c,&e))                        \
            panic("pop on an empty circular queue\n");          \
        return e;                                               \
    }                                                           \
                                                                \
                                                                \
    /* non-blocking push: returns 0 if full.*/                  \
    static inline int pfx ## _push(CQ_T *c, E_T x) {      \
        if(pfx ## _full(c))                                  \
            return 0;                                           \
        gcc_mb();                                               \
        unsigned head = c->head;                                \
        /* must occur in order */                               \
        c->c_buf[head] = x;  /* 1 */                            \
        gcc_mb();                                               \
        c->head = (head + 1) % (N); /* 2 */                     \
        gcc_mb();                                               \
        return 1;                                               \
    }                                                           \
                                                                \
    /* not thread safe in current form: need int disable */     \
    static inline E_T *pfx ## _get(CQ_T *c, unsigned i) { \
        if(i >= pfx ## _cnt(c))                            \
            return 0;                                           \
        unsigned idx = (c->tail + i) % N;                       \
        return &c->c_buf[idx];                                  \
    }                                                           \
                                                                \
    static inline void pfx ## _push_w_drop(CQ_T *q, E_T e) {    \
        /* discard oldest if full */                            \
        if(pfx ## _full(q)) {                                \
            pfx ## _pop(q);                                  \
        }                                                       \
        if(! pfx ## _push(q,e))                              \
            panic("impossible\n");                              \
    }                                                           \
                                                                \
    static inline CQ_T pfx ## _mk(void) { \
        CQ_T cq = {0};                                     \
        cq.fence = 0x12345678;                                  \
        cq.errors_fatal_p = 1;                     \
        assert(pfx ## _empty(&cq));                          \
        assert(!pfx ## _full(&cq));                          \
        assert(pfx ## _cnt(&cq) == 0);                     \
        return cq;                                              \
    }

#undef NAME

#if 0

    /* blocking: called from non-interrupt code. */             \
    static inline E_T pfx ## _cq_pop(CQ_T *c) {              \
        E_T e;                                                    \
        memset(&e, 0, sizeof e);                                \
                                                                \
	    /* wait til interrupt puts something: deadlocks if  */  \
        /* ints not enabled: need to yield. */                  \
        while(!pfx ## _cq_pop_nonblk(c,&e)) {                   \
            if(!int_is_enabled())                               \
                panic("will deadlock: interrupts not enabled [FIXME]\n");    \
        }                                                       \
        return e;                                               \
    }                                                           \


#endif
#endif
