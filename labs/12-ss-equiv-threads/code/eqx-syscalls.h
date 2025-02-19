#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__

// should keep working if you change this number.
#define EQX_SYS_EXIT            0
#define EQX_SYS_PUTC            1
#define EQX_SYS_GET_CPSR        2

/* print a single hex number (in reg <r2>) using a string <r1> */
#define EQX_SYS_PUT_HEX       3
#define EQX_SYS_PUT_INT       4
#define EQX_SYS_PUT_PID       5

#endif
