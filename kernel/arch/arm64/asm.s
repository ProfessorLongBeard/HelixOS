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
    isb
    ret

.global __ttbr1_read
__ttbr1_read:
    mrs x0, ttbr1_el1
    ret

.global __ttbr1_write
__ttbr1_write:
    msr ttbr1_el1, x0
    isb
    ret

.global __tlb_inval_all
__tlb_inval_all:
    tlbi vmalle1
    dsb ish
    isb
    ret

.global __tlb_inval_page
__tlb_inval_page:
    tlbi vaale1, x0
    dsb sy
    isb
    ret

.global __dcache_flush_addr
__dcache_flush_addr:
    dc civac, x0
    dsb ish
    isb
    ret

.global __icache_flush_addr
__icache_flush_addr:
    ic ivau, x0
    dsb ish
    isb
    ret

.global __flush_cache_range
__flush_cache_range:
    stp x2, x3, [sp, #-16]! // Preserve x2, and x3

    and x2, x0, #~(64 - 1)  // Align start address to 64-byte cache line

1:
    dc cvau, x2             // Clean and invalidate data cache by virt addr to PoU
    ic ivau, x2             // Invalidate instruction cache by virt addr to PoU
    add x2, x2, #64         // Move to the next cache line
    cmp x2, x1              // Check if we've reached the end address
    blo 1b                  // If not then continue

    dsb ish
    isb

    ldp x2, x3, [sp], #16   // Restore x2, and x3
    ret




.global __hcf
__hcf:
    wfi
    b __hcf