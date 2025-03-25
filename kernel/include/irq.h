#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>





void irq_register(int irq, void (*irq_handler)(void));

#endif