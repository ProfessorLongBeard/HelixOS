#include <kstring.h>








char *strncpy(char *dst, const char *src, size_t length) {
    char *d = (char *)dst;
    char *s = (char *)src;



    for (size_t i = 0; i < length; i++) {
        d[i] = s[i];
    }

    return dst;
}