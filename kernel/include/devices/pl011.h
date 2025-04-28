#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <mm/vmm.h>


#define UART_BASE   (VMM_VIRT_BASE + 0x09000000)

#define UART_IRQ    33


#define UART_CTS    (1UL << 0UL)
#define UART_DSR    (1UL << 1UL)
#define UART_DCD    (1UL << 2UL)
#define UART_BUSY   (1UL << 3UL)
#define UART_RXFE   (1UL << 4UL)
#define UART_TXFF   (1UL << 5UL)
#define UART_RXFF   (1UL << 6UL)
#define UART_TXFE   (1UL << 7UL)
#define UART_RI     (1UL << 8UL)

#define UART_BRK    (1UL << 0UL)
#define UART_PEN    (1UL << 1UL)
#define UART_EPS    (1UL << 2UL)
#define UART_STP2   (1UL << 3UL)
#define UART_FEN    (1UL << 4UL)

#define UART_WLEN_5BITS (0UL << 5UL)
#define UART_WLEN_6BITS (1UL << 5UL)
#define UART_WLEN_7BITS (2UL << 5UL)
#define UART_WLEN_8BITS (3UL << 5UL)

#define UART_SPS    (1UL << 7UL)

#define UART_LBE    (1UL << 7UL)
#define UART_TXE    (1UL << 8UL)
#define UART_RXE    (1UL << 9UL)
#define UART_DTR    (1UL << 10UL)
#define UART_RTS    (1UL << 11UL)
#define UART_OUT1   (1UL << 12UL)
#define UART_OUT2   (1UL << 13UL)
#define UART_RTSE   (1UL << 14UL)
#define UART_CTSE   (1UL << 15UL)

#define UART_EN     (1UL << 0UL)
#define UART_SIRED  (1UL << 1UL)
#define UART_SIRLP  (1UL << 2UL)

#define UART_RXIFLSEL   (2UL << 3UL)
#define UART_TXIFLSEL   (2UL << 0UL)

#define UART_RXIC  (1 << 4)
#define UART_TXIC  (1 << 5)

#define UART_RXIFLSEL_POS    6
#define UART_TXIFLSEL_POS    4 
#define UART_RXIFLSEL_MASK   (0x7 << UART_RXIFLSEL_POS)
#define UART_TXIFLSEL_MASK   (0x7 << UART_TXIFLSEL_POS)











typedef struct {
    uint32_t    uart_dr;                // 0x000 - UART data register
    uint32_t    uart_rsr_ecr;           // 0x004 - UART status/error clear register
    uint8_t     __reserved1[16];        // 0x008 - 0x018 reserved
    uint32_t    uart_fr;                // 0x018 - UART flag register
    uint32_t    __reserved2;            // 0x01C - reserved
    uint32_t    uart_lpr;               // 0x020 - UART low-power counter register
    uint32_t    uart_ibrd;              // 0x024 - UART integer baudrate register
    uint32_t    uart_fbrd;              // 0x028 - UART fractal baudrate register
    uint32_t    uart_lcrh;              // 0x02C - UART line control register
    uint32_t    uart_cr;                // 0x030 - UART control register
    uint32_t    uart_ifls;              // 0x034 - UART interrupt FIFO level select register
    uint32_t    uart_imsc;              // 0x038 - UART interrupt mask set/clear register
    uint32_t    uart_ris;               // 0x03C - UART raw interrupt status register
    uint32_t    uart_mis;               // 0x040 - UART masked interrupt status register
    uint32_t    uart_icr;               // 0x044 - UART interrupt clear register
    uint32_t    uart_macr;              // 0x048 - UART DMA control register
} __attribute__((packed)) uart_t;








void uart_init(void);
void uart_putc(char ch);
void uart_puts(char *s);
void uart_irq_handler(void);

#endif