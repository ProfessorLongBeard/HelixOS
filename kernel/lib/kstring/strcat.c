#include <kstring.h>
#include <kstdlib.h>





char *strcat(char *dst, const char *src) {
    if (dst == NULL || src == NULL) {
        return dst;
    }

    while(*dst != '\0') {
        dst++;
    }

    while(*src != '\0') {
        *dst = *src;
        dst++;
        src++;
    }

    *dst = '\0';
    return dst;
}