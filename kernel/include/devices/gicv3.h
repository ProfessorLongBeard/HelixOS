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

#define ICC_PMR_ALL         0xFF

#define ICC_CTLR_CBPR_S         (1UL << 0UL)
#define ICC_CTLR_CPBR_NS        (1UL << 1UL)
#define ICC_CTLR_EOI_MODE       (1UL << 1UL)
#define ICC_CTLR_EOI_S          (1UL << 3UL)
#define ICC_CTLR_EOI_NS         (1UL << 4UL)
#define ICC_CTLR_RM             (1UL << 5UL)
#define ICC_CTLR_PMHE           (1UL << 6UL)
#define ICC_CTLR_ID_16BIT       (0UL << 11UL)
#define ICC_CTLR_ID_24BIT       (1UL << 11UL)
#define ICC_CTLR_SEIS           (1UL << 14UL)
#define ICC_CTLR_A3V            (1UL << 15UL)
#define ICC_CTLR_NDS            (1UL << 17UL)
#define ICC_CTLR_RSS            (1UL << 18UL)
#define ICC_CTLR_EXT_RANGE      (1UL << 19UL)

#define ICC_IGRPEN0_ENABLE  (1UL << 1UL)
#define ICC_IGRPEN1_ENABLE  (1UL << 1UL)

#define ICC_SRE_ENABLE      (1UL << 0UL)
#define ICC_SRE_DFB_ENABLE  (1UL << 1UL)
#define ICC_SRE_DIB_ENABLE  (1UL << 2UL)

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

#define GICR_WAKER_CPU_SLEEP        (1UL << 1UL)
#define GICR_WAKER_CHILDREN_SLEEP   (1UL << 2UL)

#define GICR_CTLR_ENABLE_LPI        (1UL << 0UL)
#define GICR_CTLR_CES               (1UL << 1UL)
#define GICR_CTLR_IR                (1UL << 2UL)
#define GICR_CTLR_RWP               (1UL << 3UL)
#define GICR_CTLR_DPG0              (1UL << 24UL)
#define GICR_CTLR_DPGINS            (1UL << 25UL)
#define GICR_CTLR_DPGIS             (1UL << 26UL)
#define GICR_CTLR_UPW               (1UL << 31UL)




#define GICD_CTLR               (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0000))                // Distributor control register
#define GICD_TYPER              (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0004))                // Interrupt controller type register
#define GICD_IIDR               (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0008))                // Distributor implementer identification register
#define GICD_TYPER2             (*(volatile uint32_t *)(GIC_DIST_BASE + 0x000C))                // Interrupt controller type 2 register
#define GICD_STATUSR            (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0010))                // Error reporting status register
#define GICD_SETSPI_NSR         (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0040))                // Set SPI register (non-secure mode)
#define GICD_CLRSPI_NSR         (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0048))                // Clear SPI register (non-secure mode)
#define GICD_SETSPI_SR          (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0050))                // Set SPI register (secure mode)
#define GICD_CLRSPI_SR          (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0058))                // Clear SPI register (secure mode)
#define GICD_IGROUPR(x)         (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0080 + (x) * 4))      // Interrupt group register
#define GICD_ISENABLER(x)       (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0100 + (x) * 4))      // Interrupt set-enable register
#define GICD_ICENABLER(x)       (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0180 + (x) * 4))      // Interrupt clear-enable register
#define GICD_ISPENDR(x)         (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0200 + (x) * 4))      // Interrupt set-pending register
#define GICD_ICPENDR(x)         (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0280 + (x) * 4))      // Interrupt clear-ppending register
#define GICD_ISACTIVER(x)       (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0300 + (x) * 4))      // Interrupt set-active register
#define GICD_ICACTIVER(x)       (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0380 + (x) * 4))      // Interrupt clear-active register
#define GICD_IPRIORITYR(x)      (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0400 + (x) * 4))      // Interrupt priority register
#define GICD_ITARGETSR(x)       (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0800 + (x) * 4))      // Interrupt processor targets register
#define GICD_ICFGR(x)           (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0C00 + (x) * 4))      // Interrupt configuration register
#define GICD_IGRPMODR(x)        (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0D00 + (x) * 4))      // Interrupt group modifier register
#define GICD_NSACR(x)           (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0E00 + (x) * 4))      // Non-secure access control register
#define GICD_SGIR               (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0F00))                // Software generated interrupt register
#define GICD_CPENDSGIR(x)       (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0F10 + (x) * 4))      // SGI clear-pending register
#define GICD_SPENDSGIR(x)       (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0F20 + (x) * 4))      // SGI set-pending register
#define GICD_INMIR(x)           (*(volatile uint32_t *)(GIC_DIST_BASE + 0x0F80 + (x) * 4))      // Non-maskable interrupts
#define GICD_IROUTER(x)         (*(volatile uint32_t *)(GIC_DIST_BASE + 0x7FD8 + (x) * 4))      // Interrupt routing registers

#define GICR_RD_BASE(cpu)       (GIC_REDIST_BASE + (cpu) * 0x20000)
#define GICR_SGI_BASE(cpu)      (GICR_RD_BASE(cpu) + 0x10000)
#define GICR_CTLR(cpu)          (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x0000))
#define GICR_IIDR(cpu)          (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x0004))
#define GICR_TYPER(cpu)         (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x0008))
#define GICR_STATUSR(cpu)       (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x0010))
#define GICR_WAKER(cpu)         (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x0014))
#define GICR_MPAMIDR(cpu)       (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x0018))
#define GICR_PARTIDR(cpu)       (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x001C))
#define GICR_SETLPIR(cpu)       (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x0040))
#define GICR_CLRLIPR(cpu)       (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x0048))
#define GICR_PROPBASER(cpu)     (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x0070))
#define GICR_PENDBASER(cpu)     (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x0078))
#define GICR_INVLPIR(cpu)       (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x00A0))
#define GICR_INVALLR(cpu)       (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x00B0))
#define GICR_SYNCR(cpu)         (*(volatile uint32_t *)(GICR_RD_BASE(cpu) + 0x00C0))
#define GICR_IGROUP0(cpu)       (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0080))
#define GICR_ISENABLER0(cpu)    (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0100))
#define GICR_ICENABLER0(cpu)    (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0180))
#define GICR_ISPENDR0(cpu)      (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0200))
#define GICR_ICPENDR0(cpu)      (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0280))
#define GICR_ISACTIVER0(cpu)    (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0300))
#define GICR_ICACTICER0(cpu)    (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0380))
#define GICR_IPRIORITYR(cpu, x) (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0384 + (x) * 4))
#define GICR_ICFGR0(cpu)        (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0C00))
#define GICR_ICFGR1(cpu)        (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0C04))
#define GICR_IGRPMODR(cpu)      (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0D00))
#define GICR_NSACR(cpu)         (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0E00))
#define GICR_NIMIR0(cpu)        (*(volatile uint32_t *)(GICR_SGI_BASE(cpu) + 0x0F80))












void gic_init(void);
void gic_enable_irq(int irq);
void gic_disable_irq(int irq);
void gic_set_irq_priority(int irq, uint32_t priority);
bool gic_irq_pending(int irq);
int gic_ack_irq(void);
void gic_clear_irq(int irq);
void gic_set_irq_group(int irq, uint32_t group);
void gic_set_irq_level_trigger(int irq);
void gic_set_irq_edge_trigger(int irq);

#endif