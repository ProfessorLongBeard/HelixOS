#include <kstring.h>





char *strcpy(char *dst, const char *src) {
    char *d = (char *)dst;
    char *s = (char *)src;


    while(*s != '\0') {
        *d = *s;

        s++;
        d++;
    }

    return dst;
}