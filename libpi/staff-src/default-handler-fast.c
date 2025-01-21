#include "rpi.h"
void fast_interrupt_vector(unsigned pc) { 
    panic("unhandled fast interrupt: pc=%x\n", pc);
}
void fiq_vector(unsigned pc) { 
    panic("unhandled fast interrupt: pc=%x\n", pc);
}
