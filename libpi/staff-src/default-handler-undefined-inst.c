#include "rpi.h"
void undefined_instruction_vector(unsigned pc) { 
    panic("unhandled undefined inst exception: pc=%x\n", pc);
}
