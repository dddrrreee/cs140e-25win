#include "rpi.h"
void data_abort_vector(unsigned pc) { 
    panic("unhandled data abort: pc=%x\n", pc);
}
