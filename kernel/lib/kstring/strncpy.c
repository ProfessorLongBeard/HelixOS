#include <kstring.h>








char *strncpy(char *dst, const char *src, size_t length) {
    size_t i = 0;
    char *d = (char *)dst;
    char *s = (char *)src;


    for (i = 0; i < length && s[i] != '\0'; i++) {
        d[i] = s[i];
    }

    for (; i < length; i++) {
        d[i] = '\0';
    }

    return dst;
}