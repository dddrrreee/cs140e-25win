#ifndef __IS_MEM_H__
#define __IS_MEM_H__

// return load or store.
enum { MEM_LOAD_P = 1, MEM_STORE_P = 2 };

static inline int is_mem_inst(uint32_t pc, uint32_t enc) {
    // some insts don't have these, but if they do, these fields
    // are always in the same place.
    unsigned OP = bits_get(enc, 25, 27);
    unsigned L  = bit_get(enc, 20);         // load=1 or store=0

    // load=1 or store=0
    int type = L ?  MEM_LOAD_P : MEM_STORE_P;

    // these are currently in the order given in the arm manual book.
    switch(OP) {
    /*************************************************************
        A5.2: first encoding:
        ld/st immediate:
           27 = 0
           26 = 1 
           25 = 0

        L(20) specifies load [L=1] or store [L=0]
     */
    case 0b010:
        return type;

    /*************************************************************
        A5.2 second and third encodings.

        ld/st reg off [this shares 25..27 media, and with arch defined] 
            27 = 0
            26 = 1 
            25 = 1 *
            4  = 0 [otherwise media or arch defined]
        
        other encodings as above.
      */
    case 0b011:
        // we assume we have a mem inst: the both instructions on A5-19
        // have bit4=0
        if(bit_get(enc, 4) != 0) {
            return 0;
            // panic("should ony get here for a memory inst: bit(4) != 0\n");
        }

        return type;


    /*************************************************************
     * A5.3: misc load/store.
          [load / store halfword or double word]
      
        bits:
            27=0
            26=0
            25=0
            
        requires: bit(7)=1 and bit(4)=1.

        S(6)=0 and H(5)=0: implies a different kind of instruction

        bit(22) is 0 in one and 1 in other encoding so i think we
        can ignore it.

     */
    case 0b000:
        // both must be 1: some other inst type.
        if(!bit_get(enc, 7) || !bit_get(enc, 4)) {
            // panic("impossible: pc=%x\n", pc);
            return 0;
        }

        unsigned S = bit_get(enc, 6);
        unsigned H = bit_get(enc, 5);

        // some other encoding [tho could be ldrx and swap: 
        // need to handle]
        if(!S && !H)
            panic("not handling this case\n");

        // the L bit does not act normally.
        unsigned LSH = L << 2 | S << 1 | H;
        // A5-34
        // possible issue: double word, don't know which one
        // is loaded first.   we could set watchpoints 
        // above and below: maybe the right approach in general.
        int load_p = 0, nbytes = 0;
        switch(LSH) {
        case 0b001: load_p = 0; nbytes = 2; break;
        case 0b010: load_p = 1; nbytes = 8; break;
        case 0b011: load_p = 0; nbytes = 8;  break;
        case 0b101: load_p = 1; nbytes = 2; break;
        // signed [does this matter? 
        case 0b110: load_p = 1; nbytes = 1; break;
        // signed.
        case 0b111: load_p = 1; nbytes = 2; break;
        default: panic("illegal combination?\n");
        }
        (void)nbytes;
        return load_p ?  MEM_LOAD_P : MEM_STORE_P;

    /*************************************************************
        A5.4: ld/st multiple
            27 = 1
            26 = 0 
            25 = 0
        
            Reg list = 0..15

        - U(23) bit controls if transfer is made upwards [U=1] 
          or downwards [U=0]
        - P(24) inc/dec happens before [P=1] or after [P=0]
     */
    case 0b100:
        return type;
    default:
        return 0;
    }
    return 0;
}

#endif
