.section .text,"ax"
.global _start




_start:
    b helix_init


halt:
    wfi
    b halt