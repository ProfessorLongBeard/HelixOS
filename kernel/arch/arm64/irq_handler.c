#include <kstdio.h>
#include <kstdlib.h>
#include <arch.h>
#include <irq.h>
#include <devices/gicv3.h>



static void (*irq_handler_table[MAX_IRQ_ID])(void) = {0};




void irq_register(int irq, void (*irq_handler)(void)) {
    if (irq > MAX_IRQ_ID) {
        printf("Invalid IRQ: %d! Can't register IRQ handler\n", irq);
        return;
    }

    irq_handler_table[irq] = irq_handler;
}

void irq_handler(void) {
    int iar_irq_id = gic_ack_irq();

    if (iar_irq_id != 1023) {
        if (irq_handler_table[iar_irq_id] != NULL) {
            // Handle IRQ
            irq_handler_table[iar_irq_id]();
        } else {
            printf("IRQ ID: %d handler unregistered!\n", iar_irq_id);
            return;
        }
    } else {
        printf("No valid IRQ ID's found!\n");
        return;
    }

    gic_clear_irq(iar_irq_id);
}