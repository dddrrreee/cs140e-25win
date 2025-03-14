#ifndef __QUEUE_T_H__
#define __QUEUE_T_H__

// engler, cs140e: brain-dead generic queue. 
//
// head/tail queue, with a single next pointer.
// 
// - pfx: prepend to all helper routines.
//
// - Q_T: the type name of the struct that contains the queue (head,tail).
// - <head>: name of field to hold head.
// - <tail>: name of field to hold tail.
//
// - E_T: the type of the queue element 
// - <next>: name of field to hold the next element
#define gen_queue_T(pfx, Q_T, head, tail, E_T, next)     \
    /* iterator support. */                                             \
    static inline E_T *pfx ## _start(Q_T *q) { return q->head; }   \
    static inline E_T *pfx ## _next(E_T *e)  { return e->next; }   \
                                                                        \
    static int inline pfx ## _empty(Q_T *q)  {                          \
        if(q->head)                                                \
            return 0;                                                   \
        demand(!q->tail, invalid Q);                               \
        return 1;                                                       \
    }                                                                   \
                                                                        \
    /* remove from front of list. */                                    \
    static inline E_T *pfx ## _pop(Q_T *q) {                            \
        demand(q, bad input);                                           \
        E_T *e = q->head;                                          \
        if(!e) {                                                        \
            assert(pfx ## _empty(q));                                   \
            return 0;                                                   \
        }                                                               \
        q->head= e->next;                                    \
        if(!q->head)                                               \
            q->tail= 0;                                           \
        return e;                                                       \
    }                                                                   \
                                                                        \
    /* insert at tail. (for FIFO) */                                    \
    static inline void pfx ## _append(Q_T *q, E_T *e) {                 \
        e->next = 0;                                                    \
        if(!q->tail)                                                    \
            q->head= q->tail = e;                                      \
        else {                                                          \
            q->tail->next = e;                                          \
            q->tail= e;                                                \
        }                                                               \
    }                                                                   \
                                                                        \
    /* insert at head (for LIFO) */                                     \
    static inline void pfx ## _push(Q_T *q, E_T *e) {                   \
        e->next = q->head;                                              \
        q->head = e;                                                    \
        if(!q->tail)                                                    \
            q->tail = e;                                                \
        }                                                               \
                                                                        \
    static inline void pfx ## _init(Q_T *q) {                           \
        q->head = q->tail = 0;                                          \
    }                                                                  

#endif
