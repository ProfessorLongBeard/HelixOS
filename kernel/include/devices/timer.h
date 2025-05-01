#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>


#define TIMER_IRQ   30
#define VTIMER_IRQ  27

#define CNTV_CTL_ENABLE     (1UL << 0UL)
#define CNTV_CTL_IMASK      (1UL << 1UL)
#define CNTV_CTL_ISTATUS    (1UL << 2UL)






void timer_disable(void);
void timer_enable(void);
void timer_irq_handler(void);
uint64_t timer_get_freq(void);
uint64_t timer_phys_get_time(void);
uint64_t timer_virt_get_time(void);
void timer_set(uint64_t ms);
void timer_set_cntp(uint64_t cntp);
uint64_t timer_read_cntp(void);
uint64_t timer_phys_get_cval(void);
uint64_t timer_get_sys_ticks(void);
void timer_reset_sys_ticks(void);

#endif