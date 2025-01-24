// single header file code to setup/use the armv6 performance
// monitor unit (PMU).  note: the r/pi does not propagate
// the PMU interrupt, unfortunately, so there is no way to 
// do a sampling profiler by using it.
#ifndef __ARMV6_PMU_H__
#define __ARMV6_PMU_H__

#include "bit-support.h"
#include "asm-helpers.h"

// use the <cp_asm> macro to define the different 
// get/set PMU instructions.
cp_asm(pmu_cycle, p15, 0, c15, c12, 1)
cp_asm(pmu_event0, p15, 0, c15, c12, 2)
cp_asm(pmu_event1, p15, 0, c15, c12, 3)
cp_asm(pmu_control, p15, 0, c15, c12, 0)

// p 3-134 arm1176.pdf: configure the PMU control register
static inline void pmu_control_config(uint32_t in) {
    // 3-134 says to clear these.

    // you have to *set* these bits to clear the event.
    in = bits_set(in, 8, 10, 0b111);
    assert(bits_get(in, 8,10) == 0b111);

    pmu_control_set(in);
}

// get the type of event0 by reading the type
// field from the PMU control register and 
// returning it.
static inline uint32_t pmu_type0(void) {
    uint32_t r = pmu_control_get();
    return bits_get(r, 20, 27);
}

// set PMU event0 as <type> and enable it.
static inline void pmu_enable0(uint32_t type) {
    assert((type & 0xff) == type);

    uint32_t r = pmu_control_get();
    r = bits_set(r, 20, 27, type);
    pmu_control_config(r | 0b11);

    assert(pmu_type0() == type);
}

// get the type of event1 by reading the type
// field from the PMU control register and 
// returning it.
static inline uint32_t pmu_type1(void) {
    uint32_t r = pmu_control_get();
    return bits_get(r, 12, 19);
}

// set event1 as <type> and enable it.
static inline void pmu_enable1(uint32_t type) {
    assert((type & 0xff) == type);

    uint32_t r = pmu_control_get();
    r = bits_set(r, 12, 19, type);
    pmu_control_config(r | 0b11);

    assert(pmu_type1() == type);
}

// wrapper so can pass in the PMU register number.
static inline void pmu_enable(unsigned n, uint32_t type) {
    if(n==0)
        pmu_enable0(type);
    else if(n == 1)
        pmu_enable1(type);
    else
        panic("bad PMU coprocessor number=%d\n", n);
}

// wrapper so can pass in the PMU register number.
static inline uint32_t pmu_event_get(unsigned n) {
    if(n==0)
        return pmu_event0_get();
    else if(n == 1)
        return pmu_event1_get();
    else
        panic("bad PMU coprocessor number=%d\n", n);
}

// measure and print the count of <type0> and <type1>
// events when running the statements <stmts>.
//
// we could make it so it records the current
// and then restores it.  not sure if this 
// makes any sense.
#define pmu_stmt_measure_set(cnt0, cnt1, msg, type0, type1, stmts) do { \
    pmu_ ## type0 ## _on(0);                            \
    pmu_ ## type1 ## _on(1);                            \
    uint32_t ty0 = pmu_ ## type0(0);                    \
    uint32_t ty1 = pmu_ ## type1(1);                    \
    uint32_t cyc = pmu_cycle_get();                     \
                                                        \
    /* partially stop gcc from moving stuff */          \
    gcc_mb();                                           \
    stmts;                                              \
    gcc_mb();                                           \
                                                        \
    uint32_t n_ty0 = pmu_ ## type0(0) - ty0;            \
    uint32_t n_ty1 = pmu_ ## type1(1) - ty1;            \
    uint32_t n_cyc = pmu_cycle_get() -  cyc;            \
    const char *s0 = pmu_ ## type0 ## _str();           \
    const char *s1 = pmu_ ## type1 ## _str();           \
                                                        \
    cnt0 = n_ty0;                                       \
    cnt1 = n_ty1;                                       \
                                                        \
    output("%s:%d: %s:\n\tcycles=%d\n\t%s=%d\n\t%s=%d\n", \
        __FILE__, __LINE__, msg, n_cyc, s0, cnt0, s1, cnt1);              \
} while(0)

#define pmu_stmt_measure(msg, type0, type1, stmts) do {     \
    uint32_t c0 __attribute__((unused));                    \
    uint32_t c1 __attribute__((unused));                    \
    pmu_stmt_measure_set(c0,c1,msg, type0, type1, stmts);   \
} while(0)

// define the different event enums values.
enum {
#   define XX(fn,name,val,string) name = val,
#   define PMU_GET_TYPES

#   include "armv6-pmu.h"

#   undef PMU_GET_TYPES
#   undef XX
};

// generate the different event wrappers for setting
// reading.
#   define XX(fn_name,type,val,string)                      \
    /* enable event <type> */                               \
    static void pmu_ ## fn_name ## _on(unsigned n)          \
        { pmu_enable(n, type); }                            \
    /* read event <type> counter */                         \
    static uint32_t pmu_ ## fn_name(unsigned n)             \
        { return pmu_event_get(n); }                        \
    /* get printable string of event <type>*/               \
    static const char * pmu_ ## fn_name ## _str(void)       \
        { return string; }

#   define PMU_GET_TYPES
#   include "armv6-pmu.h"
#   undef XX
#   undef PMU_GET_TYPES

#endif

#ifdef PMU_GET_TYPES
// see 3-139 in arm1176.pdf
XX(ret_miss,        PMU_RET_MISPREDICTED,    0x26, "return mispredicted")
XX(ret_hit,         PMU_RET_PREDICTED,       0x25, "return predicted")
XX(ret_cnt,         PMU_RET_CNT,             0x24, "return count")
// the addresss was pushed on the return stack.
// there is also a 0x24 where it is popped off the 
// stack.  idk difference.
XX(call_cnt,        PMU_CALL_CNT,            0x23, "call count")
XX(main_tlb_miss,   PMU_TLB_MISS,            0x16, "TLB miss")
XX(wb_drain, PMU_WBACK_DRAINED,       0x12, "writeback drained (stall?)")
// data cache miss, noncacheable, write through.
XX(data_access,     PMU_EXPLICIT_DATA_ACCESS,0x10, "explicit data access")

XX(tlb_miss,        PMU_MAIN_TLB_MISS,       0xF,  "main tlb miss")
// i think this is all indirect jumps
XX(pc_write,        PMU_PC_WRITE,            0xD,  "sw changed pc [indir jump?]")
XX(dcache_wb,       PMU_DCACHE_WB,           0xC,  "dcache write back")
XX(dcache_miss,     PMU_DCACHE_MISS,         0xB,  "dcache miss")
XX(dcache_access,   PMU_DCACHE_ACCESS,       0xA,  "dcache access")
XX(inst_cnt,        PMU_INST_CNT,            0x7,  "instruction count")
XX(branch_miss,     PMU_BRANCH_MISPREDICT,   0x6,  "branch mispredict")
XX(branch_cnt,      PMU_BRANCH_EXECUTED,     0x5,  "branch executed")
XX(dtlb_miss, PMU_DATA_MICROTLB_MISS,  0x4,  "data microtlb miss")
XX(itlb_miss, PMU_INST_MICROTLB_MISS,  0x3,  "instruction microtlb miss")
XX(data_stall,      PMU_DATA_STALL,          0x2,  "data dependency stall")
XX(inst_stall,      PMU_INST_STALL,          0x1,  "instruction stall [tlb/cache miss]")
XX(icache_miss,     PMU_ICACHE_MISS,         0x0,  "icache miss")
XX(cycle_cnt,       PMU_CYCLE_CNT,           0xff, "cycle count")
#endif

