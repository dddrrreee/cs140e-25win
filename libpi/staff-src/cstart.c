#include "rpi.h"
#include "cycle-count.h"
#include "memmap.h"
#include "redzone.h"

// called to get the absolute end of the memory used
// by the program: later in the quarter we will append
// data to the end of the .bin file so the symbol
// <__heap_start__> (see <libpi/memmap>) won't be the
// actual end.  in that case you'll have to make
// a version of <cstart.c> that knows how to find the
// true end.  for now, we can just return 
// <__heap_start__>
void *program_end(void) {
    return __heap_start__;
}

// called to setup the C runtime and initialize commonly
// used subsystems.
//
// a nice thing about C is that it is lightweight enough 
// that setting up "the C runtime environment" involves
// just zeroing out the bss section (where zero-initialized
// static var and globals live).
void _cstart() {
	void notmain();

    // zero out the bss section.  if you look in the 
    // linker script <libpi/memmap> you can see where
    // the symbols <__bss_start__> and <__bss_end__>
    // are defined.
    //
    // NOTE: we are doing something extremely illegal 
    // --- walking from one global/static var to another 
    // by repeatedly incrementing an out-of-bounds pointer.   
    // C does not bless this practice: we are violating
    // guarantees the standard gives the compiler that it 
    // might have relied on for optimizations.  to mitigate
    // this, we add a memory barrier to prevent 
    // these stores from being pushed forward or
    // backwards in the code.  we also disable strict 
    // aliasing in gcc (linus has a rant on this).
    //
    // there's many things we do in an OS that violate
    // the C language contract so we have to be more careful
    // than when writing vanilla user code.
    gcc_mb();
    uint8_t * bss      = (void*)__bss_start__;
    uint8_t * bss_end  = (void*)__bss_end__;
    while( bss < bss_end )
        *bss++ = 0;
    gcc_mb();

    // initialize the UART device.  currently 
    // default init to 115200 baud.
    //
    // NOTE: if this program was bootloaded, then the 
    // UART is already on, but we do it anyway to make
    // sure and so that the program can run off the SD
    // card.  
    uart_init();

    // have to initialize the cycle counter or it will
    // silently return 0 when read (versus crashing).
    // pretty common to hit this multiple times in the
    // quarter so we do it here so it can't be forgotten.
    // i don't think any downside.
    cycle_cnt_init();

    // hack to catch errors where they write to the first
    // 4k of memory.
    redzone_init();

    // call user's <notmain> (should add argv)
    notmain(); 

    // if they return and don't exit, just reboot
	clean_reboot();
}
