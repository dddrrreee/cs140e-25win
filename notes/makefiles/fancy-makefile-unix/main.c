#include "rpi.h"

void foo(void);
void foo2(void);
void bar(void);
void a(void);
void b(void);
void baz(void);

int main(void){ 
    a();
    b();
    foo();      // in foo/foo.c
    // foo2();     // in foo.c
    bar(); 
    baz();
    printk("hello world\n");
    return 0;
}
