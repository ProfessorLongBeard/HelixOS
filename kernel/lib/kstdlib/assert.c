#include <kstdio.h>
#include <kstdlib.h>
#include <arch.h>







void __assert(const char *cond, const char *func, const char *file, uint32_t line) {
    printf("Assertion failed: %s: %s()@%s:%u\n", cond, func, file, line);
    while(1);
}