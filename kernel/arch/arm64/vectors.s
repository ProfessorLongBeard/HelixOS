.section .text,"ax"
.global _vector_table



.balign 2048
_vector_table:
    // ELx using SP_EL0
    b .

.balign 0x80
    b .

.balign 0x80
    b .

.balign 0x80
    b .

    // ELx using SP_ELx
.balign 0x80
    b .

.balign 0x80
    b .

.balign 0x80
    b .

    // Lower EL in aarch64 mode
.balign 0x80
    b .

.balign 0x80
    b .

.balign 0x80
    b .

.balign 0x80
    b .

    // Lower EL in aarch32 mode
.balign 0x80
    b .

.balign 0x80
    b .

.balign 0x80
    b .

.balign 0x80
    b .