#ifndef __UNIX_BOOT_H__
#define __UNIX_BOOT_H__
#ifndef __RPI__
#   include "libunix.h"
#else
#   include "rpi.h"
#endif
#include "boot-defs.h"

// hack to trace put/get.
// if 
//  <trace_p> = 1 
// you can see the stream of put/gets: makes it easy 
// to compare your bootloader to the ours and others.
//
// there are other ways to do this --- this is
// clumsy, but simple.
//
// NOTE: we could (but don't) intercept puts/gets transparently
// by interposing on a file, a socket or giving the my-install
// a file descriptor to use.
enum { TRACE_FD = 21 };
enum { TRACE_CONTROL_ONLY = 1, TRACE_ALL = 2 };
extern int trace_p;

// <addr> = pi address to put the code at [buf, buf+n).
void simple_boot(int fd, uint32_t addr, const uint8_t *buf, unsigned n);

#endif
