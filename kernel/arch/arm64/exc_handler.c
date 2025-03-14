#include <kstdio.h>
#include <stdint.h>







int exc_handler(uint64_t esr, uint64_t far, uint64_t spsr) {
    printf("Exception: esr = 0x%lx, far = 0x%lx, spsr = 0x%lx\n", esr, far, spsr);
    
    __hcf();
}