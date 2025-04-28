#include <kstdio.h>
#include <kstdlib.h>
#include <irq.h>
#include <arch.h>
#include <devices/gicv3.h>
#include <devices/timer.h>




static uint64_t timer_freq_hz = 0;
static uint64_t saved_ms = 0;






void timer_init(void) {
    timer_disable();
    timer_freq_hz = timer_get_freq();

    // TODO: Print additional information for timer?
    printf("Timer frequency: %lluHz\n", timer_freq_hz);

    irq_register(VTIMER_IRQ, timer_irq_handler);
    gic_set_irq_group_ns(VTIMER_IRQ);
    gic_set_irq_level_trigger(VTIMER_IRQ);
    gic_enable_irq(VTIMER_IRQ);
}

void timer_set(uint64_t ms) {
    uint64_t ticks = (timer_freq_hz * ms) / 1000;
    uint64_t curr_time = timer_virt_get_time();
    uint64_t deadline = curr_time + ticks;

    // Save deadline for timer reload (for now)
    saved_ms = deadline;

    // Set timer
    __cntv_cval_write(deadline);

    // Enable timer
    timer_enable();
}

void timer_disable(void) {
    uint64_t cntv = __cntv_ctl_read();
    cntv &= ~(CNTV_CTL_ENABLE);
    cntv |= CNTV_CTL_IMASK;

    if (cntv & CNTV_CTL_ISTATUS) {
        // Clear ISTATUS
        cntv &= ~(CNTV_CTL_ISTATUS);
    }

    __cntv_ctl_write(cntv);
}

void timer_enable(void) {
    uint64_t cntv = __cntv_ctl_read();
    cntv |= CNTV_CTL_ENABLE;
    cntv &= ~(CNTV_CTL_IMASK);
    __cntv_ctl_write(cntv);
}

uint64_t timer_virt_get_time(void) {
    uint64_t time = __cntvct_read();

    return time;
}

uint64_t timer_get_freq(void) {
    uint64_t timer_freq = __cntfrq_read();

    return timer_freq;
}

void timer_irq_handler(void) {
    timer_disable();

    // TODO: Improve timer handling
    __cntv_cval_write(saved_ms);

    timer_enable();
}