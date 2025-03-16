#include <kstdio.h>
#include <stdint.h>






void exc_handler(uint64_t esr, uint64_t far, uint64_t spsr) {
    printf("*** KERNEL PANIC***\n\n");
    __hcf();
}