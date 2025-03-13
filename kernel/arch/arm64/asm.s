.section .text,"ax"


.global __sctlr_read
__sctlr_read:
    mrs x0, sctlr_el1
    ret

.global __sctlr_write
__sctlr_write:
    msr sctlr_el1, x0
    ret

.global __mair_read
__mair_read:
    mrs x0, mair_el1
    ret

.global __mair_write
__mair_write:
    msr mair_el1, x0
    ret

.global __tcr_read
__tcr_read:
    mrs x0, tcr_el1
    ret

.global __tcr_write
__tcr_write:
    msr tcr_el1, x0
    ret

.global __ttbr0_read
__ttbr0_read:
    mrs x0, ttbr0_el1
    ret

.global __ttbr0_write
__ttbr0_write:
    msr ttbr0_el1, x0
    ret

.global __ttbr1_read
__ttbr1_read:
    mrs x0, ttbr1_el1
    ret

.global __ttbr1_write
__ttbr1_write:
    msr ttbr1_el1, x0
    ret





.global __hcf
__hcf:
    wfi
    b __hcf