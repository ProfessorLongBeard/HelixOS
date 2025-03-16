.section .text,"ax"
.global _start




_start:
    // Ensure we're running as core #0
    mrs x15, mpidr_el1
    and x15, x15, #0x3
    cmp x15, #0
    bne __hcf

    // Set SP_EL1 stack
    mrs x15, spsel
    orr x15, x15, #(1 << 0)
    msr spsel, x15

    // Setup initial stack space
    ldr x5, =__estack
    mov sp, x5

    // Initialize vbar_el1
    adrp x15, _vector_table
    msr vbar_el1, x15

    b helix_init
    b .