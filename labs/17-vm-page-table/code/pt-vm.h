#ifndef __VM_H__
#define __VM_H__

#include "mem-attr.h"
#include "armv6-vm.h"
#include "mmu.h"

#include "procmap.h"

// bad design: we need <pin_t>
#include "pinned-vm.h"

// from last lab.
#include "switchto.h"
#include "full-except.h"

typedef fld_t vm_pt_t;

// page table entry (confusingly same type since its an array)
typedef fld_t vm_pte_t;

static inline vm_pte_t vm_pte_mk(void) {
    // all unused fields can have 0 as default.
    return (vm_pte_t){ .tag = 0b10 };
}


// 4gb / 1mb = 4096 entries fully populated for first level
// of page table.
enum { PT_LEVEL1_N = 4096 };


// allocate zero-filled page table with correct alignment.
//  - for today's lab: assume fully populated 
//    (nentries=PT_LEVEL1_N)
vm_pt_t *vm_pt_alloc(unsigned nentries);
vm_pt_t *staff_vm_pt_alloc(unsigned nentries);

vm_pte_t *vm_map_sec(vm_pt_t *pt, uint32_t va, uint32_t pa, pin_t attr);
vm_pte_t *staff_vm_map_sec(vm_pt_t *pt, uint32_t va, uint32_t pa, pin_t attr);

// these don't do much today: exactly the same as pinned
// versions.
void vm_mmu_enable(void);
void vm_mmu_disable(void);

// mmu can be off or on.
void vm_mmu_switch(vm_pt_t *pt, uint32_t pid, uint32_t asid);

// initialize the hardware mmu and set the 
// domain reg [not sure that makes sense to do
// but we keep it b/c we did for pinned]
enum { dom_all_access = ~0, dom_no_access = 0 };

void vm_mmu_init(uint32_t domain_reg);


// lookup va in page table.
vm_pte_t * vm_lookup(vm_pt_t *pt, uint32_t va);
vm_pte_t * staff_vm_lookup(vm_pt_t *pt, uint32_t va);

// manually translate virtual address <va> to its 
// associated physical address --- keep the same
// offset.
vm_pte_t *vm_xlate(uint32_t *pa, vm_pt_t *pt, uint32_t va);
vm_pte_t *staff_vm_xlate(uint32_t *pa, vm_pt_t *pt, uint32_t va);

// allocate new page table and copy pt
vm_pt_t *vm_dup(vm_pt_t *pt);

// do an identy map for the kernel.  if enable_p=1 will
// turn on vm.
vm_pt_t *vm_map_kernel(procmap_t *p, int enable_p);
vm_pt_t *staff_vm_map_kernel(procmap_t *p, int enable_p);

// arm-vm-helpers: print <f>
void vm_pte_print(vm_pt_t *pt, vm_pte_t *pte);

// set the <AP> permissions in <pt> for the range [va, va+1MB*<nsec>) in 
// page table <pt> to <perm>
void vm_mprotect(vm_pt_t *pt, unsigned va, unsigned nsec, pin_t perm);


#define mem_attr_TEX(m) bits_get(m,2,4)
#define mem_attr_B(m) bit_get(m,0)
#define mem_attr_C(m) bit_get(m,1)



#endif
