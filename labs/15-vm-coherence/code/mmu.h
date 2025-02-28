#ifndef __SIMPLE_MMU_H__
#define __SIMPLE_MMU_H__
#include "armv6-cp15.h"

// low level MMU hardware routines.  agnostic
// as to whether we use a page table or pinning.
//
// workflow:
//  1. init mmu before doing anything.
//  2. then setup:
//   - domain permissions
//   - the asid
//   - page table (invalid or valid)
//   - any pinned entries.
// 3. then can turn the mmu on.
// 4. then can read/write memory.
// 5. then can turn the mmu off.
//
// NOTE: there are generally single hardware instructions
// for each of the above, HOWEVER, just issuing the 
// instruction is rarely enough. there is typically
// a recipe to follow to ensure that hardware state
// is consistent.  See the upcoming vm coherence lab.

// One time hardware initialization.  
// Do before anything else!
void staff_mmu_init(void);
void mmu_init(void);

// Set the domain access control to <d>.
// do before turning MMU on!
void staff_domain_access_ctrl_set(uint32_t d);
void domain_access_ctrl_set(uint32_t d);

// get
uint32_t staff_domain_access_ctrl_get(void);
uint32_t domain_access_ctrl_get(void);


// internal routine to set the hardware state: 
//   - <asid> is address space identifier.
//   - <pt> is the 2^14 aligned page table.
//
// do before turning MMU on!
void staff_set_procid_ttbr0(unsigned pid, unsigned asid, void *pt);
void set_procid_ttbr0(unsigned pid, unsigned asid, void *pt);

// called to sync up the hw state after modifying tlb entry
void staff_sync_tlb(void);

// setup pid, asid and pt in hardware.
// must call:
//  1. before turning MMU on at all
//  2. when switching address spaces (or asid won't
//     be correct).
void staff_mmu_set_ctx(uint32_t pid, uint32_t asid, void *pt);
void mmu_set_ctx(uint32_t pid, uint32_t asid, void *pt);


// called to sync after a set of pte modifications: flushes everything.
void mmu_sync_pte_mods(void);
void staff_mmu_sync_pte_mods(void);



#if 0
static inline void 
staff_mmu_set_ctx(uint32_t pid, uint32_t asid, void *pt) {
    assert(asid!=0);
    assert(asid<64);
    staff_set_procid_ttbr0(pid, asid, pt);
}

static inline void 
mmu_set_ctx(uint32_t pid, uint32_t asid, void *pt) {
    assert(asid!=0);
    assert(asid<64);
    set_procid_ttbr0(pid, asid, pt);
}
#endif

// turn the MMU on: 
// you must have previously done:
//  - <mmu_init> or hardware could have garbage.
//  - <mmu_set_ctx>: or there is no asid and page 
//    table for the hw to use.
//  - <domain_access_control_set> or no domain
//    permissions will be setup and you will immediately
//    fault.
void staff_mmu_enable(void);
void mmu_enable(void);

// turn mmu off, write-back data cache if needed, etc.
void staff_mmu_disable(void);
void mmu_disable(void);

// helper routine: is mmu on?
int staff_mmu_is_enabled(void);
int mmu_is_enabled(void);


#endif
