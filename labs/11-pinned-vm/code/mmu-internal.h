#include "armv6-cp15.h"

//*************************************************************
// internal routines for <mmu.c> ignore for today.
void set_procid_ttbr0(unsigned pid, unsigned asid, void *pt);

// set the 16 2-bit access control fields and do any needed coherence.
void domain_access_ctrl_set(uint32_t d);

void set_procid_ttbr0(unsigned pid, unsigned asid, void *pt);
void staff_set_procid_ttbr0(unsigned pid, unsigned asid, void *pt);

void staff_mmu_disable_set_asm(cp15_ctrl_reg1_t c);
void staff_mmu_enable_set_asm(cp15_ctrl_reg1_t c);

//void cp15_domain_ctrl_wr(uint32_t dom_reg);
void staff_cp15_domain_ctrl_wr(uint32_t dom_reg);

void cp15_set_procid_ttbr0(uint32_t proc_and_asid, void *pt);
void staff_cp15_set_procid_ttbr0(uint32_t proc_and_asid, void *pt);

void cp15_itlb_inv(void);
void cp15_dtlb_inv(void);
void cp15_tlbs_inv(void);

void staff_mmu_reset(void);
void mmu_reset(void);

#include "mmu.h"
