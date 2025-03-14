// lie that the assembly routines take an argument so 
// we don't have to cast.
void nop_10(void *);
void nop_1(void *);
void mov_ident(void *);
void small1(void *);
void small2(void *);
void mul_all(void *);

// put all the hashes here so we can one-stop-shop
// change them as needed.
//
// probably should have a table, with a lookup
// etc.  but for now we do it low-tech.
enum {
    SMALL1_HASH     = 0x70916294,
    SMALL2_HASH     = 0x18e43fb1,
    NOP1_HASH       = 0xf7e37b42,
    NOP10_HASH      = 0xf5ae770b,
    MOV_IDENT_HASH  = 0x984d6fbf,
};
