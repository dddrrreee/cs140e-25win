#include "rpi.h"
#include "backtrace.h"

#define bt_debug(args...) do { } while(0)

static inline int in_code(uint32_t addr) {
    extern uint32_t __code_start__, __code_end__;
    uint32_t *pc = (void*)addr;
    return pc >= &__code_start__  && pc < &__code_end__;
}

struct bt {
    struct bt *next;
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;

    const char *name;
};

static void print_ent(const char *msg, struct bt *bt) {
    output("pc=%x, lr=%x, sp=%x, next=%x\n", bt->pc, bt->lr, bt->sp, bt->next);
}

static uint32_t first_inst(struct bt *bt) {
    return bt->pc - 12;
}

static int valid_first_inst(struct bt *bt) {
    uint32_t pc = first_inst(bt);
    uint32_t x = GET32(pc);

    // if this is not true, we stop looking?
    enum { expected = 0xe1a0c00d };
    if(x == expected)
        return 1;

#if 0
    x = GET32(pc + 4 * 4);
    if(x == expected) {
        output("gcc did a zero optimization\n");
        return 1;
    }
#endif

    debug("invalid instruction at pc=%x: have %x, expected %x\n", pc, x, expected);
    return 0;
}

// does this have to be volatile so the compiler does not screw us?
const char *name_f(uint32_t pc) {
    uint32_t x = GET32(pc - 4);
    // i think most of upper bits should be 0
    uint32_t len = x & 0xffffff;
    uint32_t tag = x >> 24;
    if(tag != 0xff) {
        bt_debug("invalid length? %x [tag=%x]at pc=%x\n", len, tag, pc);
        return 0;
    }
    // debug("len=%d\n", len);

    const char *str = (void*)(pc - 4 - len);
    // output("string = %s\n", str);
    if(str[len-1] != 0)
        panic("string is not terminated\n");

    // should bound this so don't inf loop
    unsigned n = strlen(str);
    if(n > len-1)
        panic("strlen(%s)=%d, expected <=%d\n", str, n,len-1);
    return str;
}

struct bt *bt_next(struct bt *bt) {
    assert(bt->next);
    return (void*)(((uint32_t *)bt->next) - 3);
}

enum { MAX_BT = 32 };

static unsigned bt_traverse(struct bt *bt, struct bt bv[], unsigned n) {
    assert(n < MAX_BT);

    bt = bt_next(bt);
    if(!bt->next) {
        // output("done b/c null\n");
        return n;
    }
    
    // first one better be ok.
    if(!valid_first_inst(bt)) {
        output("done b/c invalid instruction\n");
        return n;
    } else {
        bv[n] = *bt;
        bv[n].name = name_f(first_inst(bt));
        // output("%d: name=<%s>:", n, name_f(first_inst(bt)));
        // print_ent("", bt);
        return bt_traverse(bt,bv, n+1);
    }
}
static void indent(unsigned n) {
    while(n-->0)
        output(" ");
}



static inline int (between)(uint32_t pc, uint32_t lo, uint32_t hi) {
    assert(hi>lo);
    return pc >= lo && pc < hi;
}
#define mk_u32(x) ((uint32_t)(x))
#define between(x,y,z) (between)(mk_u32(x), mk_u32(y), mk_u32(z))

const char *bt_recover_name(uint32_t *pc) {
    extern void memcpy_end();
    if(between(pc, memcpy, memcpy_end))
        return "memcpy [inferred]";
    return 0;
}

// given the frame pointer, return the name
const char *backtrace_name_fp(uint32_t *fp) {
    struct bt *bt = (void*)(fp-3);

    if(!valid_first_inst(bt)) {
        output("bad first inst\n");
        return 0;
    }
    return name_f(first_inst(bt));

}

// will walk all the way up.   not sure.
void backtrace_print(void) {
    // don't do a backtrace if we crash during a backtrace
    static int in_backtrace_p = 0;
    if(in_backtrace_p)
        return;
    in_backtrace_p = 1;
    
    uint32_t *fp = __builtin_frame_address (0);
    struct bt *bt = (void*)(fp-3);

    struct bt btv[MAX_BT];

    // print_ent("first bt", bt);
    
    // first one better be ok.
    if(!valid_first_inst(bt))
        panic("invalid\n");

    const char *name = name_f(first_inst(bt));
#if 0
    gcc_mb();
    output("name=<%s>\n", name);
    gcc_mb();

    // something is wrong here.  interesting bug. 

    // wait: we put this in and things don't work.  does the back
    // trace run into this when walking back?
    const char *fn = __FUNCTION__;
    if(strcmp(name, "backtrace__print") != 0)
        panic("invalid first routine: expected backtrace_print, have <%s>\n", 
                name);
#endif


    int n = bt_traverse(bt, btv, 0);
    assert(n);
    for(int i = 0; i < n; i++) {
        int off = n - i - 1 ;
        assert(off>=0);

        indent(i*2);

        if(i)   
            output("--> %d: ", i);
        struct bt *bt = &btv[off];
        output("<%s>:  pc=%x, lr=%x, sp=%x\n", 
                bt->name, bt->pc, bt->lr, bt->sp);
    }
    // unsigned n = bt_print(bt,0);

    in_backtrace_p = 0;
}
