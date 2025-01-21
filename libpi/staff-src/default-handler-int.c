#include "rpi.h"
void int_vector(unsigned pc) { 
    panic("unhandled interrupt: pc=%x\n", pc);
}
