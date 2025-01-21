#include "rpi.h"
void prefetch_abort_vector(unsigned pc) { 
    panic("unhandled prefetch abort pc=%x\n", pc);
}
