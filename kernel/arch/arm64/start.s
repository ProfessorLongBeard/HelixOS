.section .text,"ax"
.global _start




_start:
    // Ensure we're running as core #0
    mrs x15, mpidr_el1
    and x15, x15, #0x3
    cmp x15, #0
    bne __hcf

    // Initialize vbar_el1
    adrp x15, _vector_table
    msr vbar_el1, x15

    b helix_init