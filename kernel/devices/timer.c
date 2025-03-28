#include <kstdio.h>
#include <kstdlib.h>
#include <irq.h>
#include <arch.h>
#include <devices/gicv3.h>
#include <devices/timer.h>




static uint64_t timer_freq_hz = 0;




void timer_init(void) {
    timer_disable();
    timer_freq_hz = timer_get_freq();

    printf("Timer frequency: %lluHz\n", timer_freq_hz);

    irq_register(VTIMER_IRQ, timer_irq_handler);
    gic_set_irq_group_ns(VTIMER_IRQ);
    gic_set_irq_level_trigger(VTIMER_IRQ);
    gic_set_irq_priority(VTIMER_IRQ, 0xF8);
    gic_enable_irq(VTIMER_IRQ);
}

void timer_set(uint64_t ms) {
    uint64_t ticks = (timer_freq_hz * ms) / 1000;
    uint64_t curr_time = timer_virt_get_time();
    uint64_t deadline = curr_time + ticks;

    __cntv_cval_write(deadline);

    timer_enable();
}

void timer_disable(void) {
    uint64_t cntv = __cntv_ctl_read();
    cntv &= ~(CNTV_CTL_ENABLE);
    cntv |= CNTV_CTL_IMASK;
    __cntv_ctl_write(cntv);
}

void timer_enable(void) {
    uint64_t cntv = __cntv_ctl_read();
    cntv |= CNTV_CTL_ENABLE;
    cntv &= ~(CNTV_CTL_IMASK);
    __cntv_ctl_write(cntv);
}

uint64_t timer_phys_get_time(void) {
    uint64_t time = __cntpct_read();

    return time;
}

uint64_t timer_virt_get_time(void) {
    uint64_t time = __cntvct_read();

    return time;
}

uint64_t timer_get_freq(void) {
    uint64_t timer_freq = __cntfrq_read();

    return timer_freq;
}

uint64_t timer_phys_get_cval(void) {
    uint64_t cval = 0;

    __asm__ volatile("mrs %0, cntp_cval_el0\n\t" : : "r"(cval));
    return cval;
}

void timer_irq_handler(void) {
    printf("Timer IRQ handler called!\n");

    __icc_eoir1_write(TIMER_IRQ);
}