#include "rpi.h"

void foo(void);
void foo2(void);
void bar(void);
void a(void);
void b(void);

void notmain(void){ 
    a();
    b();
    foo();      // in foo/foo.c
    foo2();     // in foo.c
    bar(); 
    printk("hello world\n");
}
