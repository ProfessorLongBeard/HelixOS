.section .vectors,"ax"
.global _vector_table


.equ SVC_CALL,  0x15






.macro __build_stack_frame null
    // Save general purpous registers
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x8, x9, [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, x19, [sp, #-16]!
    stp x20, x21, [sp, #-16]!
    stp x22, x23, [sp, #-16]!
    stp x24, x25, [sp, #-16]!
    stp x26, x27, [sp, #-16]!
    stp x28, x29, [sp, #-16]!
    stp x30, xzr, [sp, #-16]!

    // Save return address
    mrs x0, elr_el1
    stp x0, x1, [sp, #-16]!
.endm

.macro __restore_stack_frame null
    // Restore return address
    ldp x0, xzr, [sp], #16
    msr elr_el1, x0

    // Restore general purpous registers
    ldp x30, xzr, [sp], #16
    ldp x28, x29, [sp], #16
    ldp x26, x27, [sp], #16
    ldp x24, x25, [sp], #16
    ldp x22, x23, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x18, x19, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8, x9, [sp], #16
    ldp x6, x7, [sp], #16
    ldp x4, x5, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16
.endm














.align 11
_vector_table:
.align 7
    // ELx using SP_EL0
    b __aarch64_syncronous_handler

.align 7
    b .

.align 7
    b .

.align 7
    b .

    // ELx using SP_ELx
.align 7
    b __aarch64_syncronous_handler

.align 7
    b .

.align 7
    b .

    // Lower EL in aarch64 mode
.align 7
    b __aarch64_syncronous_handler

.align 7
    b .

.align 7
    b .

.align 7
    b .

    // Lower EL in aarch32 mode
.align 7
    b .

.align 7
    b .

.align 7
    b .

.align 7
    b .








__aarch64_syncronous_handler:
    __build_stack_frame

    mrs x0, esr_el1
    mrs x1, far_el1
    mrs x2, spsr_el1
    bl exc_handler

    __restore_stack_frame
    eret