#include <kstdio.h>
#include <arch.h>
#include <stdbool.h>
#include <devices/gicv3.h>



/*
 * Software Generated Interrupt INTID's: 0 - 15
 * Private Peripheral Interrupt INTID"s: 16 - 31
 * Shared Peripheral Interrupt INTID's:  32 - 1029
 * Special Interrupt Number(s) INTID's:  1020 - 1023
 * Reserved INTID's:                     1024 - 8191
 * Locality Peripheral Interrupt INTID:  8192+
 *
 * GIC Distributor & Re-distributor are used to configure interrupts while GIC CPU interface is used to handle them
 */

 /*
  * ICC_IAR0_EL1: Used to ACK group 0 interrupts (read in FIQ handler)
  * ICC_IAR1_EL1: Used to ACK group 1 interrupts (read in IRQ handler)
  * ICC_NMIAR1_EL1: Used to ACK non-maskable group 1 interrupts (GIC v3.3 only)
  *
  * ICC_SGI0R_EL1: Generate secure group 0 software interrupt
  * ICC_SGI1R_EL1: Generate group 1 software interrupt for current security state
  * ICC_ASGI1R_EL1: Gerneates group 1 software interrupt for other security states
  */






void gic_init(void) {
    GICD_CTLR = GICD_CTLR_ENABLE_GRP0 | GICD_CTLR_ENABLE_GRP1_NS | GICD_CTLR_ENABLE_GRP1_S | GICD_CTLR_ENABLE_ARE_S | GICD_CTLR_ENABLE_ARE_NS;

    // Wake GIC CPU interface
    uint32_t waker = GICR_WAKER;
    waker &= ~(GICR_WAKER_CPU_SLEEP);
    GICR_WAKER = waker;

    while(GICR_WAKER & GICR_WAKER_CPU_SLEEP);

    uint64_t sre = __icc_sre_read();
    sre |= 1;
    __icc_sre_write(sre);

    uint32_t ctlr = __icc_ctlr_read();
    ctlr |= ICC_CTLR_EOI_MODE;
    __icc_ctlr_write(ctlr);

    // Set default priority
    __icc_pmr_write(0xA0);

    __icc_brp1_write(0b111);

    uint64_t grp1 = __icc_igrpen1_read();
    grp1 |= 1;
    __icc_igrpen1_write(grp1);

    __daif_clr();
    printf("GIC: Initialized!\n");
}

int gic_ack_irq(void) {
    int iar = __icc_iar1_read();

    return iar;
}

void gic_clear_irq(int irq) {
    __icc_eoir1_write(irq);
}

void gic_enable_irq(int irq) {
    int reg = irq / 32;
    int bit = irq % 32;

    GICD_ISENABLER(reg) = (1UL << bit);
}

void gic_disable_irq(int irq) {
    int reg = irq / 32;
    int bit = irq % 32;

    GICD_ICENABLER(reg) = (1UL << bit);
}

bool gic_irq_pending(int irq) {
    int reg = irq / 32;
    int bit = irq % 32;

    if (GICD_ISPENDR(reg)) {
        return true;
    }

    return false;
}

void gic_set_irq_priority(int irq, uint8_t priority) {
    int reg = irq / 32;
    int shift = (irq % 4) * 8;
    uint32_t val = 0;

    val = GICD_IPRIORITYR(reg);

    val &= ~(0xFF << shift);
    val |= (priority << shift);

    GICD_IPRIORITYR(reg) = val;
}

void gic_trigger_irq(int irq) {
    uint32_t sgi_val = (0UL << 16UL) | (irq);
    GICD_SGIR = sgi_val;
}