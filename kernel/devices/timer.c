#include <kstdio.h>
#include <kstdlib.h>
#include <irq.h>
#include <arch.h>
#include <devices/gicv3.h>
#include <devices/timer.h>
#include <spinlock.h>






static spinlock_t s;
static volatile uint64_t timer_freq_hz = 0;
static volatile uint64_t saved_ticks = 0;
static volatile uint64_t sys_ticks = 0;






void timer_init(void) {
    timer_disable();
    timer_freq_hz = timer_get_freq();

    spinlock_init(&s);

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
    saved_ticks = ticks;



    spinlock_acquire(&s);

    // Disable timer
    timer_disable();

    // Set timer
    __cntv_cval_write(deadline);

    // Enable timer
    timer_enable();

    spinlock_release(&s);
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

uint64_t timer_get_sys_ticks(void) {
    return sys_ticks;
}

void timer_reset_sys_ticks(void) {
    spinlock_acquire(&s);

    // Reset sys_tick count
    sys_ticks = 0;

    spinlock_release(&s);
}

void timer_sleep(uint64_t delay) {
    uint64_t start = timer_get_sys_ticks();


    spinlock_acquire(&s);

    while((timer_get_sys_ticks() - start) < delay) {
        __asm__ volatile("wfi\n\t");
    }

    spinlock_release(&s);
}

void timer_irq_handler(void) {
    uint64_t curr_time = timer_virt_get_time();
    uint64_t deadline = curr_time + saved_ticks;
    

    spinlock_acquire(&s);

    timer_disable();

    sys_ticks++;

    __cntv_cval_write(deadline);

    timer_enable();

    spinlock_release(&s);
}