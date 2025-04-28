#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <stddef.h>
#include <mm/vmm.h>


#define RTC_BASE    (VMM_VIRT_BASE + 0x09010000)

#define RTC_IRQ_ID  29

#define RTC_TIMER_FREQ  32768

#define RTCCR_ENABLE    (1UL << 0UL)    // Enable RTC
#define RTCIMSC_MASK    (1UL << 0UL)    // Set or clear interrupt mask
#define RTCICR_CLEAR    (1UL << 0UL)    // Clear interrupt




#define RTCDR       (*(volatile uint32_t *)(RTC_BASE + 0x000))      // RTC data register
#define RTCMR       (*(volatile uint32_t *)(RTC_BASE + 0x004))      // RTC match register
#define RTCLR       (*(volatile uint32_t *)(RTC_BASE + 0x008))      // RTC load register
#define RTCCR       (*(volatile uint32_t *)(RTC_BASE + 0x00C))      // RTC control register
#define RTCIMSC     (*(volatile uint32_t *)(RTC_BASE + 0x010))      // Interrupt mask set or clear register
#define RTCRIS      (*(volatile uint32_t *)(RTC_BASE + 0x014))      // Raw interrupt status register
#define RTCMIS      (*(volatile uint32_t *)(RTC_BASE + 0x018))      // Masked interrupt status
#define RTCICR      (*(volatile uint32_t *)(RTC_BASE + 0x01C))      // Interrupt clear register







void rtc_init(void);
void rtc_irq_handler(void);

#endif