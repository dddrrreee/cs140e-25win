#ifndef __EQX_SYSCALLS_H__
#define __EQX_SYSCALLS_H__

// should keep working if you change this number.
#define EQX_SYS_EXIT            0
#define EQX_SYS_PUTC            1


#define EQX_SYS_FORK        2
#define EQX_SYS_EXEC        3
#define EQX_SYS_WAITPID     4
#define EQX_SYS_SBRK        5

#if 0
#define EQX_SYS_OPEN        6
#define EQX_SYS_CLOSE       7
#define EQX_SYS_READ        8
#define EQX_SYS_WRITE       9
#define EQX_SYS_DUP         10
#endif

#define EQX_SYS_ABORT       11

// not Unix core syscalls.
// #define EQX_SYS_PUTC            128
#define EQX_SYS_GET_CPSR        129

// printing: provided so the test binary is small.
#define EQX_SYS_PUT_HEX         130 
#define EQX_SYS_PUT_INT         131
#define EQX_SYS_PUT_PID         132

#define EQX_SYS_MAX             256

#define WNOHANG  (1 << 2)
#endif
