#include "rpi.h"

// probably should make len an int so can catch sign
// errors.
void safe_strcpy(char *dst, const char *src, unsigned n) {
    if(!n)
        return;

    // copy at most <n>-1 bytes.
    for(int i = 0; i < n-1; i++) {
        // if we hit the terminator, return.
        if(!(dst[i] = src[i]))
            return;
    }
    dst[n-1] = 0;
}

