#ifndef ARCH_H
#define ARCH_H

#include <stdint.h>


#define DAIF_FIQ    (1UL << 6UL)
#define DAIF_IRQ    (1UL << 7UL)
#define DAIF_SERR   (1UL << 8UL)
#define DAIF_DMASK  (1UL << 9UL)

#define MAIR_ATTR(attr, idx) ((uint64_t)(attr) << (8ULL * idx))

#define MT_NORMAL           0U
#define MT_NORMAL_NC        2U
#define MT_DEVICE_NGNRNE    3U
#define MT_DEVICE_NGNRE     4U

#define NORMAL_MEMORY       0xFF
#define NORMAL_MEMORY_NC    0x44
#define DEVICE_NGNRNE       0x00
#define DEVICE_NGNRE        0x04

#define SCTLR_MMU           (1UL << 0UL)
#define SCTLR_ALIGNMENT     (1UL << 1UL)
#define SCTLR_DCACHE        (1UL << 2UL)
#define SCTLR_ICACHE        (1UL << 12UL)
#define SCTLR_WXN           (1UL << 19UL)
#define SCTLR_SPAN          (1UL << 23UL)
#define SCTLR_EOE           (1UL << 24UL)
#define SCTLR_EE            (1UL << 25UL)


#define TCR_T0SZ_SHIFT      0
#define TCR_T1SZ_SHIFT      16
#define TCR_SZ_MASK         0x3F

#define TCR_T0SZ_SET        (x) (((x) & TCR_SZ_MASK) << TCR_T0SZ_SHIFT)
#define TCR_T1SZ_SET        (x) (((x) & TCR_SZ_MASK) << TCR_T1SZ_SHIFT)

#define TCR_EPD0            (1UL << 7UL)

#define TCR_IRGN0_NORMAL_MEMORY_NC      (0UL << 8UL)
#define TCR_IRGN0_NORMAL_MEMORY_WBWA    (1UL << 8UL)
#define TCR_IRGN0_NORMAL_MEMORY_WTWA    (2UL << 8UL)
#define TCR_IRGN0_NORMAL_MEMORY_WBNA    (3UL << 8UL)

#define TCR_ORGN0_NORMAL_MEMORY_NC      (0UL << 10UL)
#define TCR_ORGN0_NORMAL_MEMORY_WBWA    (1UL << 10UL)
#define TCR_ORGN0_NORMAL_MEMORY_WTWA    (2UL << 10UL)
#define TCR_ORGN0_NORMAL_MEMORY_WBNA    (3UL << 10UL)

#define TCR_SH0_NS      (0UL << 12UL)
#define TCR_SH0_OS      (1UL << 12UL)
#define TCR_SH0_IS      (2UL << 12UL)

#define TCR_TG0_4K      (0UL << 14UL)
#define TCR_TG0_64K     (1UL << 14UL)
#define TCR_TG0_16K     (2UL << 14UL)

#define TCR_A1          (1UL << 22UL)

#define TCR_EPD1        (1UL << 23UL)

#define TCR_IRGN1_NORMAL_MEMORY_NC      (0UL << 24UL)
#define TCR_IRGN1_NORMAL_MEMORY_WBWA    (1UL << 24UL)
#define TCR_IRGN1_NORMAL_MEMORY_WTWA    (2UL << 24UL)
#define TCR_IRGN1_NORMAL_MEMORY_WBNA    (3UL << 24UL)

#define TCR_ORGN1_NORMAL_MEMORY_NC      (0UL << 26UL)
#define TCR_ORGN1_NORMAL_MEMORY_WBWA    (1UL << 26UL)
#define TCR_ORGN1_NORMAL_MEMORY_WBTA    (2UL << 26UL)
#define TCR_ORGN1_NORMAL_MEMORY_WBNA    (3UL << 26UL)

#define TCR_SH1_NS      (0UL << 28UL)
#define TCR_SH1_OS      (1UL << 28UL)
#define TCR_SH1_IS      (2UL << 28UL)

#define TCR_TG1_16K     (1UL << 30UL)
#define TCR_TG1_4K      (2UL << 30UL)
#define TCR_TG1_64K     (3UL << 30UL)

#define TCR_IPS_32BIT   (0UL << 32UL)
#define TCR_IPS_36BIT   (1UL << 32UL)
#define TCR_IPS_40BIT   (2UL << 32UL)
#define TCR_IPS_42BIT   (3UL << 32UL)
#define TCR_IPS_44BIT   (4UL << 32UL)
#define TCR_IPS_48BIT   (5UL << 32UL)
#define TCR_IPS_52BIT   (6UL << 32UL)



extern uint64_t __sctlr_read(void);
extern void __sctlr_write(uint64_t sctlr);

extern uint64_t __mair_read(void);
extern void __mair_write(uint64_t mair);

extern uint64_t __tcr_read(void);
extern void __tcr_write(uint64_t tcr);

extern uint64_t __ttbr0_read(void);
extern void __ttbr0_write(void);

extern uint64_t __ttbr1_read(void);
extern void __ttbr1_write(uint64_t ttbr1);

extern void __tlb_inval_page(uint64_t addr);
extern void __tlb_inval_all(void);

extern void __dcache_flush_addr(uint64_t addr);
extern void __icache_flush_addr(uint64_t addr);

extern void __flush_cache_range(uint64_t start, uint64_t end);

extern uint64_t __icc_pmr_read(void);
extern void __icc_pmr_write(uint64_t pmr);
extern uint64_t __icc_sre_read(void);
extern void __icc_sre_write(uint64_t sre);
extern uint64_t __icc_igrpen1_read(void);
extern void __icc_igrpen1_write(uint64_t grp);
extern uint64_t __icc_ctlr_read(void);
extern void __icc_ctlr_write(uint64_t ctlr);
extern uint64_t __icc_iar1_read(void);
extern void __icc_eoir1_write(uint64_t eoir);

extern void __daif_set(void);
extern void __daif_clr(void);

extern void __hcf(void);

#endif