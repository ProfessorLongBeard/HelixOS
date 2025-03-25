#ifndef GICv3_H
#define GICv3_H

#include <stdint.h>
#include <stddef.h>
#include <vmm.h>
#include <stdbool.h>



#define MAX_IRQ_ID  256

#define GIC_DIST_BASE       (VMM_RECURSIVE_BASE + 0x08000000)
#define GIC_CPU_BASE        (VMM_RECURSIVE_BASE + 0x08010000)
#define GIC_REDIST_BASE     (VMM_RECURSIVE_BASE + 0x080A0000)

#define ICC_PMR_ALL     0xFF

#define ICC_SRE_ENABLE      (1UL << 0UL)
#define ICC_SRE_DFB_ENABLE  (1UL << 1UL)
#define ICC_SRE_DIB_ENABLE  (1UL << 2UL)

#define ICC_IGRPEN1_ENABLE  (1UL << 0UL)

#define ICC_CTLR_CBPR           (1UL << 0UL)
#define ICC_CTLR_EOI_MODE       (1UL << 1UL)
#define ICC_CTLR_PMHE           (1UL << 6UL)
#define ICC_CTLR_ID_16BIT       (0UL << 11UL)
#define ICC_CTLR_ID_24BIT       (1UL << 11UL)
#define ICC_CTLR_SEIS           (1UL << 14UL)
#define ICC_CTLR_A3V            (1UL << 15UL)
#define ICC_CTLR_RSS            (1UL << 18UL)
#define ICC_CTLR_EXT_RANGE      (1UL << 19UL)

#define GICD_CLRSPI_NSR_INTD(x)   ((x) << 12UL)
#define GICD_CLRSPI_SR_INTD(x)    ((x) << 12UL)

#define GICD_SGI_CLRPEND_BITS(x) (0xFF << (8 * (x)))
#define GICD_SGI_CLRPEND_BIT0    GICD_SGI_CLRPEND_BITS(0)
#define GICD_SGI_CLRPEND_BIT1    GICD_SGI_CLRPEND_BITS(1)
#define GICD_SGI_CLRPEND_BIT2    GICD_SGI_CLRPEND_BITS(2)
#define GICD_SGI_CLRPEND_BIT3    GICD_SGI_CLRPEND_BITS(3)

#define GICD_CTLR_ENABLE_GRP0       (1UL << 0UL)
#define GICD_CTLR_ENABLE_GRP1_NS    (1UL << 1UL)
#define GICD_CTLR_ENABLE_GRP1_S     (1UL << 2UL)
#define GICD_CTLR_ENABLE_ARE_S      (1UL << 4UL)
#define GICD_CTLR_ENABLE_ARE_NS     (1UL << 5UL)
#define GICD_CTLR_DISABLE_SEC       (1UL << 6UL)
#define GICD_CTLR_ENABLE_E1NWF      (1UL << 7UL)
#define GICD_CTLR_ENABLE_RWP        (1UL << 31UL)

#define GICC_CTLR_ENABLE_GRP1       (1UL << 0UL)
#define GICC_CTLR_FIQ_BYPASS_GRP1   (1UL << 5UL)
#define GICC_CTLR_IRQ_BYPASS_GRP1   (1UL << 6UL)
#define GICC_CTLR_EIO_MODE_NS       (1UL << 9UL)

#define GICR_WAKER_CPU_SLEEP        (1UL << 0UL)
#define GICR_WAKER_CHILDREN_SLEEP   (1UL << 1UL)

#define GICR_CTLR_ENABLE_LPI        (1UL << 0UL)
#define GICR_CTLR_CES               (1UL << 1UL)
#define GICR_CTLR_IR                (1UL << 2UL)
#define GICR_CTLR_RWP               (1UL << 3UL)
#define GICR_CTLR_DPG0              (1UL << 24UL)
#define GICR_CTLR_DPGINS            (1UL << 25UL)
#define GICR_CTLR_DPGIS             (1UL << 26UL)
#define GICR_CTLR_UPW               (1UL << 31UL)



#define GICD_CTLR           (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0000))                // Distributor control register
#define GICD_TYPER          (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0004))                // Interrupt controller type register
#define GICD_IIDR           (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0008))                // Distributor implementer identification register
#define GICD_TYPER2         (*(volatile uint32_t *)(GIC_DIST_BASE + 0x000C))                // Interrupt controller type 2 register
#define GICD_STATUSR        (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0010))                // Error reporting status register
#define GICD_SETSPI_NSR     (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0040))                // Set SPI register (non-secure mode)
#define GICD_CLRSPI_NSR     (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0048))                // Clear SPI register (non-secure mode)
#define GICD_SETSPI_SR      (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0050))                // Set SPI register (secure mode)
#define GICD_CLRSPI_SR      (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0058))                // Clear SPI register (secure mode)
#define GICD_IGROUPR(x)     (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0080 + (x) * 4))      // Interrupt group register
#define GICD_ISENABLER(x)   (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0100 + (x) * 4))      // Interrupt set-enable register
#define GICD_ICENABLER(x)   (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0180 + (x) * 4))      // Interrupt clear-enable register
#define GICD_ISPENDR(x)     (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0200 + (x) * 4))      // Interrupt set-pending register
#define GICD_ICPENDR(x)     (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0280 + (x) * 4))      // Interrupt clear-ppending register
#define GICD_ISACTIVER(x)   (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0300 + (x) * 4))      // Interrupt set-active register
#define GICD_ICACTIVER(x)   (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0380 + (x) * 4))      // Interrupt clear-active register
#define GICD_IPRIORITYR(x)  (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0400 + (x) * 4))      // Interrupt priority register
#define GICD_ITARGETSR(x)   (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0800 + (x) * 4))      // Interrupt processor targets register
#define GICD_ICFGR(x)       (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0C00 + (x) * 4))      // Interrupt configuration register
#define GICD_IGRPMODR(x)    (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0D00 + (x) * 4))      // Interrupt group modifier register
#define GICD_NSACR(x)       (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0E00 + (X) * 4))      // Non-secure access control register
#define GICD_SGIR           (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0F00))                // Software generated interrupt register
#define GICD_CPENDSGIR(x)   (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0F10 + (x) * 4))      // SGI clear-pending register
#define GICD_SPENDSGIR(x)   (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0F20 + (x) * 4))      // SGI set-pending register
#define GICD_INMIR(x)       (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0F80 + (x) * 4))      // Non-maskable interrupts

#ifdef GIC_v2
#define GICC_CTLR           (*(volatile uint32_t *)(GIC_CPU_BASE + 0x0000))                 // CPU interface control register
#define GICC_PMR            (*(volatile uint32_t *)(GIC_CPU_BASE + 0x0004))                 // Interrupt priority mask
#define GICC_BPR            (*(volatile uint32_t *)(GIC_CPU_BASE + 0x0008))                 // Binary point register
#define GICC_IAR            (*(volatile uint32_t *)(GIC_CPU_BASE + 0x000C))                 // Interrupt ACK register
#define GICC_EOIR           (*(volatile uint32_t *)(GIC_CPU_BASE + 0x0010))                 // End of interrupt register
#define GICC_RPR            (*(volatile uint32_t *)(GIC_CPU_BASE + 0x0014))                 // Running priority register
#define GICC_HPPIR          (*(volatile uint32_t *)(GIC_CPU_BASE + 0x0018))                 // Highest priority interrupt register
#define GICC_STATUSR        (*(volatile uint32_t *)(GIC_CPU_BASE + 0x002C))                 // Error reporting status register
#define GICC_APR(x)         (*(volatile uint32_t *)(GIC_CPU_BASE + 0x00D0 + (x) * 4))       // Active priorities register
#define GICC_NSAPR(x)       (*(volatile uint32_t *)(GIC_CPU_BASE + 0x00E0 + (x) * 4))       // Non-secure active priorities register
#define GICC_IIDR           (*(volatile uint32_t *)(GIC_CPU_BASE + 0x00FC))                 // CPU interface identification register
#define GICC_DIR            (*(volatile uint32_t *)(GIC_CPU_BASE + 0x1000))                 // Deactivate interrupt register
#endif

#define GICR_CTLR           (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0000))              // Redistributor control register
#define GICR_IIDR           (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0004))              // Implementor idenification register
#define GICR_TYPR           (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0008))              // Redistributor register type
#define GICR_STATUSR        (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0010))              // Error reporting register
#define GICR_WAKER          (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0014))              // Redistributor wake register
#define GICR_MPAMIDR        (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0018))              // Report maximum PARTID and PMG register
#define GICR_PARTIDR        (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x001C))              // Set PARTID and PMG register
#define GICR_SETLPIR        (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0040))              // Set LPI pending register
#define GICR_CLRLIPR        (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0048))              // Clear LPI pending register
#define GICR_PROPBASER      (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0070))              // Redistributor properties base address register
#define GICR_PENDBASER      (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0078))              // Redistributor LPI pending table base address register
#define GICR_INVLPIR        (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x00A0))              // Redistributor invaliate LPI register
#define GICR_INVALLR        (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x00B0))              // Redistributor invalidate all register
#define GICR_SYNCR          (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x00C0))              // Redistributor synchronize register
#define GICR_IGROUP0        (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0080))              // Interrupt group register 0
#define GICR_ISENABLER0     (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0100))              // Interrupt set-enable register 0
#define GICR_ICENABLER0     (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0180))              // Interrupt clear-enable register 0
#define GICR_ISPENDR0       (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0200))              // Interrupt set-pend register 0
#define GICR_ICPENDR0       (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0280))              // Interrupt clear-pend register 0
#define GICR_ISACTIVER0     (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0300))              // Interrupt set-active register 0
#define GICR_ICACTICER0     (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0380))              // Interrupt clear-active register 0
#define GICR_IPRIORITYR(x)  (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0384 + (x) * 4))    // Interrupt priority register
#define GICR_ICFGR0         (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0C00))              // SGI configuration register
#define GICR_ICFGR1         (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0C04))              // PPI configuration register
#define GICR_IGRPMODR       (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0D00))              // Interrupt group modifier register 0
#define GICR_NSACR          (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0E00))              // Non-secure access control register
#define GICR_NIMIR0         (*(volatile uint32_t *)(GIC_REDIST_BASE + 0x0F80))              // Non-maskable interrupt register for PPI's and SGI's












void gic_init(void);
void gic_enable_irq(int irq);
void gic_disable_irqI(int irq);
void gic_set_irq_priority(int irq, uint8_t priority);
void gic_trigger_irq(int irq);
bool gic_irq_pending(int irq);

#endif