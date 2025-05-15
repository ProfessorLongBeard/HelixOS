#include <kstring.h>





char *strpbrk(const char *s, const char *brk) {
    while(*s) {
        if (strchr(brk, *s)) {
            return (char *)s;
        }

        s++;
    }

    return NULL;
}