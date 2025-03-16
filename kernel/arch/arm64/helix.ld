OUTPUT_FORMAT("elf64-littleaarch64")

ENTRY(_start)


PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
    stack PT_LOAD;
}

SECTIONS
{
    . = 0xffffffff80000000;

    __kernel_start = .;

    .limine_requests : {
        __slimine_requests = .;
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
        __elimine_requests = .;
        __requests_size = __elimine_requests - __slimine_requests;
    } :limine_requests

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text : {
        __stext = .;
        KEEP(*(.vectors));
        *(.text .text.*)
        __etext = .;
        __text_size = __etext - __stext;
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        __srodata = .;
        *(.rodata .rodata.*)
        __erodata = .;
        __rodata_size = __erodata - __srodata;
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        __sdata = .;
        *(.data .data.*)
        __edata = .;
        __data_size = __edata - __sdata;
    } :data

    .bss : {
        __sbss = .;
        *(.bss .bss.*)
        *(COMMON)
        __ebss = .;
        __bss_size = __ebss - __sbss;
    } :data

    .stack : {
        . = ALIGN(16);
        __sstack = .;
        . = . + 0x10000; /* Initial 64KB stack space */
        __estack = .;
        __stack_size = __estack - __sstack;
    } :stack

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }

    __kernel_end = .;
}