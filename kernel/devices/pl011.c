#include <stdint.h>
#include <devices/pl011.h>




uart_t *pl011 = (uart_t *)UART_BASE;









void uart_init(void) {
    pl011->uart_cr &= ~(UART_EN);
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