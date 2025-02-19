#ifndef __SIMPLE_MMU_H__
#define __SIMPLE_MMU_H__
#include "armv6-cp15.h"


//************************************************************
// 1. init mmu before doing anything.
void staff_mmu_init(void);

//************************************************************
// 2. then setup:
//   - domain permissions
//   - the asid and pt
//   - any pinned entries.

void staff_domain_access_ctrl_set(uint32_t d);

// internal routines ignore
void staff_set_procid_ttbr0(unsigned pid, unsigned asid, void *pt);

// setup pid, asid and pt in hardware.
static inline void 
staff_mmu_set_ctx(uint32_t pid, uint32_t asid, void *pt) {
    assert(asid!=0);
    assert(asid<64);
    staff_set_procid_ttbr0(pid, asid, pt);
}

//************************************************************
// 3. then can turn the mmu on.

// turn the MMU on: you must have done an <mmu_init> first 
// or could be garbage.
void staff_mmu_enable(void);

//************************************************************
// 4. then can turn the mmu off.

void staff_mmu_disable(void);


//***********************************************************
// helper routines

// is mmu on?
#define mmu_is_enabled staff_mmu_is_enabled
int mmu_is_enabled(void);

// called to sync up the hw state after modifying tlb entry
void staff_sync_tlb(void);

#endif
