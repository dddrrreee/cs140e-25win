#include "rpi.h"
void syscall_vector(unsigned pc) { 
    panic("unhandled system call: pc=%x\n", pc);
}
