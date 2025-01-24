// should have them do things this way: once they pass checks, restructure
// to use inline routines.  everything should be the same.  have them figure
// out how to cover all the code?

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000

static const unsigned gpio_set0  = (GPIO_BASE + 0x1C);
static const unsigned gpio_clr0  = (GPIO_BASE + 0x28);
static const unsigned gpio_lev0  = (GPIO_BASE + 0x34);


static inline void put32_raw(volatile void *addr, unsigned v) {
    *(volatile uint32_t *)addr = v;
}
static inline void PUT32_raw(unsigned addr, unsigned v) {
    put32_raw((volatile void *)addr, v);
}
static inline unsigned get32_raw(const volatile void *addr) {
    return *(volatile uint32_t *)addr;
}
static inline unsigned GET32_raw(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}

static void or32_raw(volatile void *addr, unsigned v) {
    put32_raw(addr, get32_raw(addr) | v);
}

// set <pin> to output.  note: fsel0, fsel1, fsel2 are contiguous in memory,
// so you can use array calculations!

#define PUT32 "error"
#define GET32 "error"

static inline void dmb_raw(void) {
    uint32_t r0 = 0;
    asm volatile("mcr p15, 0, %0, c7, c10, 5" :: "r"(r0));
}

static inline void 
gpio_set_function_raw(unsigned pin, gpio_func_t func) {
#if 0
    if(pin >= 32)
        return;
    if((func & 0b111) != func)
        return;
#endif
    unsigned off = (pin%10)*3;
    unsigned g = GPIO_BASE + (pin/10)*4;

    unsigned v = GET32_raw(g) ;
    v &= ~(0b111 << off);
    v |= func << off;
    PUT32_raw(g,v);
}

static inline void 
gpio_set_output_raw(unsigned pin) {
    gpio_set_function_raw(pin, GPIO_FUNC_OUTPUT);
}

// set <pin> on.
static inline void 
gpio_set_on_raw(unsigned pin) {
    PUT32_raw(gpio_set0, 1 << pin);
}

// set <pin> off
static inline void 
gpio_set_off_raw(unsigned pin) {
    PUT32_raw(gpio_clr0, 1 << pin);
}

static inline void 
gpio_set_off_mask(uint32_t mask) {
    PUT32_raw(gpio_clr0, mask);
}
static inline void 
gpio_set_on_mask(uint32_t mask) {
    PUT32_raw(gpio_set0, mask);
}
static inline void 
gpio_write_mask(uint32_t mask, unsigned v) {
    if(v)
        gpio_set_on_mask(mask);
    else
        gpio_set_off_mask(mask);
}




// set <pin> to input.
static inline void 
gpio_set_input_raw(unsigned pin) {
    gpio_set_function_raw(pin, GPIO_FUNC_INPUT);
}

// set <pin> to <v> (v \in {0,1})
static inline void 
gpio_write_raw(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on_raw(pin);
    else
        gpio_set_off_raw(pin);
}

static inline int 
gpio_read_raw(unsigned pin) {
    unsigned bank  = (gpio_lev0 + pin/32);
    unsigned off = (pin%32);
    return (GET32_raw(bank) >> off) & 1;
}
static inline int 
gpio_read_mask(uint32_t mask) {
    return GET32_raw(gpio_lev0) & mask;
}

#undef PUT32
#undef GET32
