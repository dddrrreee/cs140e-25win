#ifndef __CYCLE_COUNT_H__
#define __CYCLE_COUNT_H__
// use r/pi cycle counters so that we can do tighter timings.

#ifndef RPI_UNIX
// must do init first.
static inline void cycle_cnt_init(void) {
    uint32_t in = 1;
    asm volatile("MCR p15, 0, %0, c15, c12, 0" :: "r"(in));
}

// read cycle counter: should add a write().
static inline uint32_t cycle_cnt_read(void) {
	uint32_t out;
  	asm volatile("MRC p15, 0, %0, c15, c12, 1" : "=r"(out));
    return out;
}

#else

// if we are doing fake-pi testing, have to provide fake 
// versions of these.
void cycle_cnt_init(void);
unsigned cycle_cnt_read(void);

#endif

// some helper macros to make measuring the cycle count of
// different operations easier.
#define TIME_CYC(_fn) ({                \
    unsigned _s = cycle_cnt_read();     \
    _fn;                                \
    unsigned _e = cycle_cnt_read();     \
    ((_e - _s) - 0);                    \
})

#define TIME_CYC_10(_fn) ({                     \
    unsigned _s = cycle_cnt_read();             \
    _fn; _fn; _fn; _fn; _fn;                    \
    _fn; _fn; _fn; _fn; _fn;                    \
    unsigned _e = cycle_cnt_read();             \
    ((_e - _s)-0);                              \
})

#define TIME_CYC_PRINT(_msg, _fn) do {                          \
    unsigned _t = TIME_CYC(_fn);                                \
    printk("%s: %d cycles <%s>\n", _msg, _t, _XSTRING(_fn));    \
} while(0)

#define TIME_CYC_PRINT10(_msg, _fn) do {                            \
    unsigned _t = TIME_CYC_10(_fn);                                 \
    printk("%s: %d cycles (x10) <%s>\n", _msg, _t, _XSTRING(_fn));  \
} while(0)

#endif
