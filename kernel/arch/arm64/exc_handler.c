#include <kstdio.h>
#include <kstdlib.h>
#include <arch/esr.h>






void exc_handler(uint64_t esr, uint64_t far, uint64_t spsr) {
    printf("Synchronous exception: 0x%08X\n\n", far);

    while(1) { __asm__ volatile("nop\n\t"); }
}