#include "rpi.h"
void reset_vector(unsigned pc) { 
    panic("unhandled reset exception: pc=%x\n", pc);
}
