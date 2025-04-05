#include <kstring.h>





char *strchr(const char *s, char c) {
    while(*s != '\0') {
        if (*s == c) {
            return (char *)s;
        }

        s++;
    }

    if (c == '\0') {
        return (char *)s;
    }

    return NULL;
}