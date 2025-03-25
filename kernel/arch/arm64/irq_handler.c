#include <kstdio.h>
#include <kstdlib.h>
#include <arch.h>
#include <irq.h>
#include <devices/gicv3.h>



static void (*irq_handler_table[MAX_IRQ_ID])(void) = {0};




void irq_register(int irq, void (*irq_handler)(void)) {
    assert(irq <= MAX_IRQ_ID);

    irq_handler_table[irq] = irq_handler;
}

void irq_handler(void) {
    int irq = __icc_iar1_read();
    assert(irq <= MAX_IRQ_ID);

    printf("IRQ: Got IRQ ID: %d! Exectuting IRQ vector\n", irq);

    if (irq_handler_table[irq]) {
        irq_handler_table[irq]();
    } else {
        printf("IRQ: %d not registered!\n", irq);
    }
}