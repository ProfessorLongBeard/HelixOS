#include <kstdio.h>
#include <arch.h>
#include <stdbool.h>
#include <devices/gicv3.h>



/*
 * Software Generated Interrupt INTID's: 0 - 15
 * Private Peripheral Interrupt INTID's: 16 - 31
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
    GICD_CTLR = GICD_CTLR_ENABLE_GRP0 | GICD_CTLR_ENABLE_GRP1_NS | GICD_CTLR_ENABLE_ARE_NS | GICD_CTLR_ENABLE_GRP1_S | GICD_CTLR_ENABLE_ARE_S;

    // Wait for register writes in distributor to take effect. This will prevent
    // any race conditions that might cause an incorrectly configured GIC
    while(GICD_CTLR & GICD_CTLR_ENABLE_RWP);

    // Wake GIC CPU interface
    uint32_t waker = GICR_WAKER(0);
    waker &= ~(GICR_WAKER_CPU_SLEEP);
    GICR_WAKER(0) = waker;

    // Wait for children to wake up
    while(GICR_WAKER(0) & GICR_WAKER_CHILDREN_SLEEP);

    uint64_t sre = __icc_sre_read();
    sre |= 1;
    __icc_sre_write(sre);

    // Allow all interrupts
    __icc_pmr_write(ICC_PMR_ALL);

    __icc_brp1_write(0b111);

    __icc_igrpen1_read();
    uint64_t grp1 = 1;
    __icc_igrpen1_write(grp1);
    
    uint32_t ctlr = __icc_ctlr_read();
    ctlr |= ICC_CTLR_CPBR_NS | ICC_CTLR_EOI_NS | ICC_CTLR_EOI_MODE;
    __icc_ctlr_write(ctlr);

    for (int irq = 0; irq < MAX_IRQ_ID; irq++) {
        gic_disable_irq(irq);
    }

    __daif_clr();
    printf("GIC: Initialized!\n");
}

int gic_ack_irq(void) {
    int iar = __icc_iar1_read();

    return iar;
}

void gic_clear_irq(int irq) {
    int reg = irq / 32;
    int bit = irq % 32;

    if (irq < 32) {
        GICR_ICPENDR0(0) = (1UL << bit);

    } else {
        GICD_ICPENDR(reg) = (1UL << bit);
    }
}

void gic_enable_irq(int irq) {
    int reg = irq / 32;
    int bit = irq % 32;


    if (irq < 32) {
        GICR_NSACR(0) |= (2UL << (bit * 2));
        GICR_ISENABLER0(0) |= (1UL << bit);
    } else {
        GICD_NSACR(reg) |= (2UL << (bit * 2));
        GICD_ISENABLER(reg) |= (1UL << bit);
    }
}

void gic_disable_irq(int irq) {
    int reg = irq / 32;
    int bit = irq % 32;

    if (irq < 32) {
        GICR_ICENABLER0(0) |= (1UL << bit);
    } else {
        GICD_ICENABLER(reg) |= (1UL << bit);
    }
}

bool gic_irq_pending(int irq) {
    int reg = irq / 32;
    int bit = irq % 32;
    uint32_t pend = 0;


    if (irq < 32) {
        pend = GICR_ICPENDR0(0);
    } else {
        pend = GICD_ICPENDR(reg);
    }

    if (pend & (1UL << bit)) {
        return true;
    }
    
    return false;
}

void gic_set_irq_priority(int irq, uint32_t priority) {
    int reg = irq / 4;
    int bit = (irq % 4) * 8;
    uint32_t curr_priority = 0;


    if (priority > 0xFF) {
        priority = 0xFF;
    }

    if (irq < 32) {
        curr_priority = GICR_IPRIORITYR(0, reg);

        curr_priority &= ~(0xFF << bit);
        curr_priority |= (priority << bit);

        GICR_IPRIORITYR(0, reg) = curr_priority;
    } else {
        curr_priority = GICD_IPRIORITYR(reg);

        curr_priority &= ~(0xFF << bit);
        curr_priority |= (priority << bit);

        GICD_IPRIORITYR(reg) = curr_priority;
    }
}

void gic_set_irq_group_ns(int irq) {
    int reg = irq / 32;
    int bit = irq % 32;


    if (irq < 32) {
        GICR_IGROUP0(0) |= (1UL << bit);
    } else {
        GICD_IGROUPR(reg) |= (1UL << bit);
    }
}

void gic_set_irq_level_trigger(int irq) {
    int reg = irq / 16;
    int bit = (irq % 16) * 2;


    if (irq >= 16 && irq < 32) {
        GICR_ICFGR1(0) &= ~(3UL << bit);
        GICR_ICFGR1(0) |= (0UL << bit);
    } else {
        GICD_ICFGR(reg) &= ~(3UL << bit);
        GICD_ICFGR(reg) |= (0UL << bit);
    }
}

void gic_set_irq_edge_trigger(int irq) {
    int reg = irq / 16;
    int bit = (irq % 16) * 2;


    if (irq >= 16 && irq < 32) {
        GICR_ICFGR1(0) &= ~(3UL << bit);
        GICR_ICFGR1(0) |= (2UL << bit);
    } else {
        GICD_ICFGR(reg) &= ~(3UL << bit);
        GICD_ICFGR(reg) |= (2UL << bit);
    }
}