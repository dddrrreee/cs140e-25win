#include "rpi-asm.h"

#define INV_ICACHE(Rd)                                           \
    mcr p15, 0, Rd, c7, c5, 0   ; /* invalidate entire I-cache */   \
    mcr p15, 0, Rd, c7, c5, 0;  ; /* invalidate entire I-cache */   \
    mcr p15, 0, Rd, c7, c5, 0;  ; /* invalidate entire I-cache */   \
    mcr p15, 0, Rd, c7, c5, 0;  ; /* invalidate entire I-cache */   \
    .rept   11                  ; /* ARM Ltd recommends at least 11 nops */\
    nop                         ;                                   \
    .endr

#define PREFETCH_FLUSH(Rd)  mcr p15, 0, Rd, c7, c5, 4  
#define FLUSH_BTB(Rd)         mcr p15, 0, Rd, c7, c5, 6
#define DSB(Rd)             mcr p15, 0, Rd, c7, c10, 4
#define DMB(Rd)             mcr p15, 0, Rd, c7, c10, 5 
#define INV_TLB(Rd)         mcr p15, 0, Rd, c8, c7, 0

#define CLEAN_INV_DCACHE(Rd)    mcr p15, 0, Rd, c7, c14, 0  

// flush everything.
MK_FN(cache_flush_all)
MK_FN(flush_caches)
    INV_TLB(r1);
    CLR(r1);
    CLEAN_INV_DCACHE(r1);  
    DSB(r1);

    INV_ICACHE(r1);  
    DSB(r1);

    FLUSH_BTB(r1);      
    PREFETCH_FLUSH(r1);     @ wait for flush btb.

    bx lr
