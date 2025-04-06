#include <kstdio.h>
#include <stdint.h>
#include <vmm.h>






void exc_handler(uint64_t esr, uint64_t far, uint64_t spsr) {
    printf("*** KERNEL PANIC ***\n");

    __hcf();
}