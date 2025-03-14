/* 
  seperate code and data:
   1. for the usual reasons of r/o code.
   2. but also so we can cut down memory needed when forking. 

  note: the header is such that we can change these without modifying the loader
  code.

od -t x4 hello.bin | less


*/
/*
CODE_ADDR =  0x100000;
DATA_ADDR =  0x200000;
*/

/* we keep code seperate from data so the code can be mapped
   read-only and shared. */

CODE_ADDR =  0x400000;

/* should probably make this 16MB aligned so that we can
   use 16MB supersections for really big processes */
DATA_ADDR =  0x500000;

MEMORY
{
    ROM  (rx) : ORIGIN = 0x0, LENGTH = 256k
    CODE (rx) : ORIGIN = CODE_ADDR, LENGTH = 64K
    DATA (rw) : ORIGIN = DATA_ADDR, LENGTH = 128K
}

/* entry point routine */
ENTRY(_start)

/* i think it's better to have the addr, offset, nbytes */

SECTIONS
{
    /* our simple a.out header.  will add as needed. */
    .crt0_header : {
        /* word 0: magic cookie for sanity checking */
        LONG(0xfaf0faf0);

        /* size of the header: sanity check. */
        LONG(SIZEOF(.crt0_header));

        /* 
         * code:
         * - word 1: where r/o text starts in file
         * - word 2: where it goes in virtual address space
         * - word 3: size in bytes
         */
        LONG(LOADADDR(.text) - ORIGIN(ROM));
        LONG(ADDR(.text));
        LONG(SIZEOF(.text));
        
        /*
         * data: 
         * - word 4: where starts in file
         * - word 5: where it goes in virtual address space
         * - word 6: size in bytes
         */
        LONG(LOADADDR(.data) - ORIGIN(ROM));
        LONG(ADDR(.data));
        LONG(SIZEOF(.data));

        /* 
         * bss: 
         *  - word 7: where goes in va
         *  - word 8: size in bytes
         */
        LONG(ADDR(.bss));
        LONG(SIZEOF(.bss));

        /* should have the size of the entire thing so can sanity check [sizeof rom?] */
        /* LONG(SIZEOF(ROM)); */

        KEEP(*(.crt0_header));
    } > ROM

    .text : {
        __text_start__ = .;
        KEEP(*(.text.boot))  
        *(.text*)
        *(.rodata*)
        . = ALIGN(8);
        __text_end__ = .;
    } > CODE AT > ROM
 
    .data : {
        __data_start__ = .;
        . = ALIGN(8);
        *(.data*)
        . = ALIGN(8);
        __data_end__  = .;
    } > DATA AT > ROM

    .bss : {
        __bss_start__ = .;
        . = ALIGN(8);
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end__ = .;
        __prog_end__ = .;
        __heap_start__ = .;
    } > DATA

    /* i don't think we are discarding everything. */

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.debug*)
        *(*.attributes*)
    }
}
