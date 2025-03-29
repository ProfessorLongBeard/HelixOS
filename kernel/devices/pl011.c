#include <stdint.h>
#include <devices/pl011.h>
#include <devices/gicv3.h>
#include <kstdio.h>
#include <irq.h>





uart_t *pl011 = (uart_t *)UART_BASE;









void uart_init(void) {
    // TODO: Get baudrate from cmdline
    uint64_t uart_divisor = 24000000 / (16 * 115200);

    pl011->uart_cr &= ~(UART_EN);

    pl011->uart_ibrd = uart_divisor & 0xFFFF;
    pl011->uart_fbrd = (uart_divisor >> 16) & 0x3F;

    pl011->uart_lcrh |= UART_WLEN_8BITS | UART_FEN | UART_PEN | UART_EPS;

    pl011->uart_fr &= ~(UART_RXIFLSEL_MASK | UART_TXIFLSEL_MASK);
    pl011->uart_fr |= (UART_RXIFLSEL << UART_RXIFLSEL_POS) | (UART_TXIFLSEL << UART_TXIFLSEL_POS);

    pl011->uart_imsc |= UART_RXIC | UART_TXIC;

    pl011->uart_cr |= UART_EN | UART_TXE | UART_RXE;

    irq_register(UART_IRQ, uart_irq_handler);
    gic_set_irq_group_ns(UART_IRQ);
    gic_set_irq_level_trigger(UART_IRQ);
    gic_enable_irq(UART_IRQ);
}

void uart_irq_handler(void) {
    printf("UART IRQ handler called!\n");
    
    if (pl011->uart_mis & UART_RXIC) {
        pl011->uart_icr = UART_RXIC;
    }

    if (pl011->uart_mis & UART_TXIC) {
        pl011->uart_icr = UART_TXIC;
    }
}

void uart_putc(char ch) {
    while(pl011->uart_fr & UART_TXFF);
    pl011->uart_dr = ch;
}

void uart_puts(char *s) {
    while(*s != '\0') {
        uart_putc(*s);
        s++;
    }
}