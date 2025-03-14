#ifndef __SMALL_PROG_H__
#define __SMALL_PROG_H__

struct prog {
    const char *name;
    unsigned nbytes;
    uint8_t code[];
};


// if we keep everything exactly as it is in small-proc.ld we
// can just memcpy it.

// can have a union of these headers, differentiated by
// magic
typedef struct {
    // these fields bit-identically in the program header
    uint32_t magic;
    uint32_t hdr_nbytes;

    uint32_t code_offset;   // offset in the exec file
    uint32_t code_addr;
    uint32_t code_nbytes;

    uint32_t data_offset;   // offset in the exec file
    uint32_t data_addr;
    uint32_t data_nbytes;

    uint32_t bss_addr;
    uint32_t bss_nbytes;

} small_prog_hdr_t;

// sanity check header.
static inline small_prog_hdr_t 
small_prog_hdr_mk(uint32_t *hdr) {
    small_prog_hdr_t h = *(small_prog_hdr_t *)hdr;

    if(h.magic != 0xfaf0faf0)
        panic("magic = %x, expected 0xfaf0faf0\n", h.magic);
    if(h.hdr_nbytes != sizeof h)
        panic("hdr size = %d nybtes, expected %d\n", 
            h.hdr_nbytes, sizeof h);
    if(h.code_offset != h.hdr_nbytes)
        panic("expected code right after header [off=%d], is at off=%d\n",
            h.hdr_nbytes,
            h.code_offset);

    // sanity
    assert(h.data_offset > h.code_offset);
    assert(h.data_addr > h.code_addr);
    assert(h.data_offset == h.code_offset + h.code_nbytes);

    assert(h.bss_addr >= h.data_addr);
    assert(h.bss_addr >= h.data_addr+h.data_nbytes);
    return h;
}

static inline void 
small_prog_hdr_print(small_prog_hdr_t s) {
    output("code offset = %d nybtes\n", s.code_offset);
    output("code addr = %x \n", s.code_addr);
    output("code size = %d bytes \n", s.code_nbytes);

    output("data offset = %d nybtes\n", s.data_offset);
    output("data addr = %x \n", s.data_addr);
    output("data size = %d  bytes \n", s.data_nbytes);

    output("bss addr = %x \n", s.bss_addr);
    output("bss size = %d bytes \n", s.bss_nbytes);
}

#if 0
// copy all the pieces out to where we want.
static inline void 
small_prog_copyto(void *text, void *data, void *bss, s) {
#endif

#endif
