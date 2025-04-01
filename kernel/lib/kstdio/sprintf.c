#include <kstdio.h>






int sprintf(char *s, const char *fmt, ...) {
    int ret = 0;
    va_list args;

    va_start(args, fmt);
    ret = sprintf_(s, fmt, args);
    va_end(args);

    return ret;
}