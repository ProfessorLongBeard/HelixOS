#include <kstdio.h>
#include <kstdlib.h>
#include <arch.h>
#include <irq.h>
#include <devices/gicv3.h>



static void (*irq_handler_table[MAX_IRQ_ID])(void) = {0};




void irq_register(int irq, void (*irq_handler)(void)) {
    assert(irq <= MAX_IRQ_ID);

    printf("Registering IRQ handler %d\n", irq);
    irq_handler_table[irq] = irq_handler;
}

void irq_handler(void) {
    // Check for higher priority IRQ's
    int irq_id = gic_ack_hppi_irq();

    if (irq_id == -1) {
        // Fallback to IAR if no higher priorty IRQ was found
        irq_id = gic_ack_irq();

        if (irq_id == -1) {
            // If no lower priority IRQ is found then bail out
            printf("No valid IRQ ID's found!\n");
            return;
        }

        if (irq_handler_table[irq_id] == NULL) {
            // Can't handle IRQ's with no registered IRQ handler
            printf("IRQ: %d unregistered!\n");
            return;
        }

        // Mask interupts and call IRQ handler
        __daif_set();
        irq_handler_table[irq_id]();

    } else {
        if (irq_handler_table[irq_id] == NULL) {
            // Can't handle IRQ's with no registered IRQ handler
            printf("IRQ: %d unregistered!\n");
            return;
        }

        // Mask interrupts and call IRQ handler
        __daif_set();
        irq_handler_table[irq_id]();
    }

    // Unmask IRQ's and return
    __daif_clr();
}