## Some puzzles

-----------------------------------------------------------------
### B2.7.7: changing privilege level.

need anything?

    CPS #USER_LEVEL

------------------------------------------------------------------
### B2.7.6 writing cp15, cp14

Need anyting?

    // write control reg 1
    mcr p15, 0, r0, c1, c0, 0

------------------------------------------------------------------
### B2.7.5 invalidating the BTB

Need anything?

    // armv6-coprocessor.h: FLUSH_BTB
    mcr p15, 0, r0, c7, c5, 6

------------------------------------------------------------------
### Self-modifying code  (B2-22)

Note: if not self-modify (where code already has run and now
we change its bits) can eliminate stuff.

Code:

        1: STR rx  ; instruction location
        2: clean data cache by MVA ; instruction location
        3: DSB  ; ensures visibility of (2)
        4: invalidate icache by MVA
        5: Invalidate BTB
        6: DSB; ensure (4) completes --- do we need for (5)?
        7: Prefetch flush

Why?


------------------------------------------------------------------
#### ASID / PT change (B2-25)

Code:

        1. Change ASID to 0
        2. PrefetchFlush
        3. change TTBR to new page table
        4. PrefetchFlush
        5. Change asid to new value.
        6. anything else ???

Why?

-----------------------------------------------------------------
#### PTE modification (b2-23)

Code:

        1: STR rx ; pte
        2: clean line ; pte
        3: DSB  ; ensure visibility of data cleaned from dcache
        4: invalidate tlb entry by MVA ; page address
        5: invalidate btb
        6: DSB; to ensure (4) completes
        7: prefetchflush

Why?









