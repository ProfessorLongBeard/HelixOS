#include <kstdio.h>








int printf(const char *fmt, ...) {
    int ret = 0;
    va_list args;


    va_start(args, fmt);
    ret = vprintf_(fmt, args);
    va_end(args);

    return ret;
}

// For ubsan (TODO: Add an ifdef to enable this, maybe?)
int print(const char *fmt, ...) {
    int ret = 0;
    va_list args;

    va_start(args, fmt);
    ret = vprintf_(fmt, args);
    va_end(args);

    return ret;
}