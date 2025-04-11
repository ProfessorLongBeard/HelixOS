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

.global __icc_sgi1r_write
__icc_sgi1r_write:
    msr icc_sgi1r_el1, x0
    isb
    ret

.global __icc_brp1_read
__icc_brp1_read:
    mrs x0, icc_bpr1_el1
    ret

.global __icc_brp1_write
__icc_brp1_write:
    msr icc_bpr1_el1, x0
    isb
    ret

.global __icc_pmr_read
__icc_pmr_read:
    mrs x0, icc_pmr_el1
    ret

.global __icc_pmr_write
__icc_pmr_write:
    msr icc_pmr_el1, x0
    isb
    ret

.global __icc_rpr_read
__icc_rpr_read:
    mrs x0, icc_rpr_el1
    ret

.global __icc_sre_read
__icc_sre_read:
    mrs x0, icc_sre_el1
    ret

.global __icc_sre_write
__icc_sre_write:
    msr icc_sre_el1, x0
    isb
    ret

.global __icc_igrpen0_read
__icc_igrpen0_read:
    mrs x0, icc_igrpen0_el1
    ret

.global __icc_igrpen0_write
__icc_igrpen0_write:
    msr icc_igrpen0_el1, x0
    isb
    ret

.global __icc_igrpen1_read
__icc_igrpen1_read:
    mrs x0, icc_igrpen1_el1
    ret

.global __icc_igrpen1_write
__icc_igrpen1_write:
    msr icc_igrpen1_el1, x0
    isb
    ret

.global __icc_ctlr_read
__icc_ctlr_read:
    mrs x0, icc_ctlr_el1
    ret

.global __icc_ctlr_write
__icc_ctlr_write:
    msr icc_ctlr_el1, x0
    ret

.global __icc_iar1_read
__icc_iar1_read:
    mrs x0, icc_iar1_el1
    ret

.global __icc_hppir_read
__icc_hppir_read:
    mrs x0, icc_HPPIR1_EL1
    ret

.global __icc_eoir1_write
__icc_eoir1_write:
    msr ICC_EOIR1_EL1, x0
    ret

.global __daif_set
__daif_set:
    msr DAIFSet, #0b1111
    ret

.global __daif_clr
__daif_clr:
    msr DAIFClr, #0b1111
    ret

.global __cntvct_read
__cntvct_read:
    mrs x0, cntvct_el0
    ret

.global __cntv_cval_read
__cntv_cval_read:
    mrs x0, cntv_cval_el0
    ret

.global __cntv_cval_write
__cntv_cval_write:
    msr cntv_cval_el0, x0
    isb
    ret

.global __cntpct_read
__cntpct_read:
    mrs x0, cntpct_el0
    ret

.global __cntp_tval_write
__cntp_tval_write:
    msr cntp_tval_el0, x0
    isb
    ret

.global __cntv_tval_read
__cntv_tval_read:
    mrs x0, cntv_tval_el0
    ret

.global __cntp_tval_read
__cntp_tval_read:
    mrs x0, cntp_tval_el0
    ret

.global __cntv_tval_write
__cntv_tval_write:
    msr cntv_tval_el0, x0
    isb
    ret

.global __cntfrq_read
__cntfrq_read:
    mrs x0, cntfrq_el0
    ret

.global __cntfrq_write
__cntfrq_write:
    msr cntfrq_el0, x0
    isb
    ret

.global __cntv_ctl_read
__cntv_ctl_read:
    mrs x0, cntv_ctl_el0
    ret

.global __cntv_ctl_write
__cntv_ctl_write:
    msr cntv_ctl_el0, x0
    isb
    ret

.global __cntp_ctl_read
    mrs x0, cntp_ctl_el0
    ret

.global __cntp_ctl_write
__cntp_ctl_write:
    msr cntp_ctl_el0, x0
    isb
    ret

.global __cntp_cval_write
__cntp_cval_write:
    msr cntp_cval_el0, x0
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


.global __mb
__mb:
    // memory barrier
    dsb sy
    ret



.global __hcf
__hcf:
    wfi
    b __hcf