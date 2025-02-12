#ifndef __PI_SYS_LOCK_H__
#define __PI_SYS_LOCK_H__

// should not be in this file, we do just to keep things simple.
typedef volatile int pi_lock_t;

static inline void sys_lock_init(pi_lock_t *l) { *l = 0; }

static inline void sys_lock(pi_lock_t *l) {
    while(!sys_lock_try(l))
        ;
    assert(*l == 1);
}

static inline void sys_unlock(pi_lock_t *l) { 
    assert(*l == 1);
    *l = 0; 
}
#endif
