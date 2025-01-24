#include "rpi.h"

// some helpers.
static inline uint32_t max_u32(uint32_t x, uint32_t y) {
    return x > y ? x : y;
}
static inline unsigned is_aligned(unsigned x, unsigned n) {
    return (((x) & ((n) - 1)) == 0);
}
static inline unsigned is_aligned_ptr(void *ptr, unsigned n) {
    return is_aligned((unsigned)ptr, n);
}
static inline unsigned is_pow2(unsigned x) { 
    return (x & -x) == x;
}
static inline unsigned roundup(unsigned x, unsigned n) {
    assert(is_pow2(n));
    return (x+(n-1)) & (~(n-1));
}



// this is the minimum alignment
union align {
        double d;
        void *p;
        void (*fp)(void);
};

// we shouldn't need more than 2mb
enum { heap_max = 1024*1024*2 };

#include "memmap.h"

static void *heap = 0;
static void *heap_start = 0;
static void *heap_end = 0;

// i don't know if we should even have these.
void *kmalloc_heap_end(void) { return heap_end; }
void *kmalloc_heap_start(void) { return heap_start; }
void *kmalloc_heap_ptr(void) { return heap; }

// set the start of the heap to <addr>
void kmalloc_init_set_start(void *_addr, unsigned max_nbytes) {
    void *addr = (void*)_addr;
    if(addr < program_end())
        panic("address is too small\n");

    assert(max_nbytes);
    if(heap_start > addr)
        panic("dangerous: creating heap that could overwrite pre-existing\n");

    // if resetting, make sure doesn't hit the old heap.
    if(addr < heap)
        panic("setting heap to %p: will hit the old heap at %p\n",
            addr, heap_end);

    heap_start = heap = addr;
    heap_end = heap_start + max_nbytes;
}

// allocate a block of size <nbytes> , don't zero it.
void *kmalloc_notzero(unsigned nbytes) {
    assert(nbytes);
    nbytes = roundup(nbytes, sizeof(union align));

    assert((unsigned)heap%8 == 0);

    void *addr = heap;
    assert(addr);

    heap += nbytes;
    if(heap > heap_end)
        panic("%d nbyte allocation exceeded size of heap by %u nbytes\n", 
            nbytes, heap - heap_end);
    return addr;
}

// alloate a block of size <nbytes>, zero it.
void *kmalloc(unsigned nbytes) {
    // make a backtrace assert
    assert(nbytes);
    void *addr = kmalloc_notzero(nbytes);
    memset(addr, 0, nbytes);
    return addr;
}

// return a block of memory of size <nbytes> and aligned to
// <alignment> (ie.  ptr % alignment == 0).
//  currently: alignment must be a power of 2.
void *kmalloc_aligned(unsigned nbytes, unsigned alignment) {
    assert(nbytes);
    demand(is_pow2(alignment), assuming power of two);
    alignment = max_u32(alignment, sizeof(union align));

    // XXX: we waste the alignment memory.  aiya.
    //  really should migrate to the k&r allocator.
    unsigned h = (unsigned)heap;
    h = roundup(h, alignment);
    demand(is_aligned(h, alignment), impossible);

    heap = (void*)h;
    return kmalloc(nbytes);
}

// frees the entire heap: set <heap> to <heap_start>
void kfree_all(void) { heap = heap_start; }
