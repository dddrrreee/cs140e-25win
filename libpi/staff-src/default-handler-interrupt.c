#include "rpi.h"
void interrupt_vector(unsigned pc) { 
    panic("unhandled interrupt: pc=%x\n",pc);
}
