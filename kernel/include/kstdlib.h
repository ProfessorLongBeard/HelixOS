#ifndef KSTDLIB_H
#define KSTDLIB_H

#include <stdint.h>
#include <stddef.h>




#define assert(x) ({\
    if (!(x)) {\
        __assert(#x, __func__, __FILE__, __LINE__);\
    }\
    })


void __assert(const char *cond, const char *func, const char *file, uint32_t line);

#endif