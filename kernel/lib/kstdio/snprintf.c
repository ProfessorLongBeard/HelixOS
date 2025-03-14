#include <kstdio.h>






int snprintf(char *s, size_t count, const char *fmt, ...) {
    int ret = 0;
    va_list args;

    va_start(args, fmt);
    ret = vsnprintf_(s, count, fmt, args);
    va_end(args);

    return ret;
}