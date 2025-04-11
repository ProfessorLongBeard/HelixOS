#include <kstdio.h>
#include <irq.h>
#include <devices/rtc.h>
#include <devices/gicv3.h>





void rtc_init(void) {
    irq_register(RTC_IRQ_ID, rtc_irq_handler);
    gic_set_irq_group_ns(RTC_IRQ_ID);
    gic_set_irq_level_trigger(RTC_IRQ_ID);
    gic_enable_irq(RTC_IRQ_ID);
}

void rtc_irq_handler(void) {
    printf("RTC IRQ handler called!\n");
}