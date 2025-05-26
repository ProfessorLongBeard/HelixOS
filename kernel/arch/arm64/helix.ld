OUTPUT_FORMAT("elf64-littleaarch64")

ENTRY(_start)


PF_R = 0x4;
PF_W = 0x2;
PF_X = 0x1;

PHDR_RW = (PF_R | PF_W);
PHDR_RX = (PF_R | PF_X);
PHDR_RO = PF_R;




PHDRS
{
    limine_requests PT_LOAD     FLAGS(PHDR_RW);
    text            PT_LOAD     FLAGS(PHDR_RX);
    rodata          PT_LOAD     FLAGS(PHDR_RO);
    data            PT_LOAD     FLAGS(PHDR_RW);
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

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .bss (NOLOAD) : {
        __sbss = .;
        *(.bss .bss.*)
        *(COMMON)
        __ebss = .;
        __bss_size = __ebss - __sbss;

        /* 256KB stack space */
        . = ALIGN(16);
        __sstack = .;
        . += 262144;
        __estack = .;
    } :data

    __kernel_end = .;

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}