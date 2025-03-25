#include <kstdio.h>
#include <devices/gicv3.h>








void irq_handler(void) {
    printf("IRQ handler called!\n");
}