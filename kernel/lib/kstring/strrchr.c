#include <kstring.h>







char *strrchr(const char *s, char ch) {
    char *ret = NULL;

    do {
        if (*s == ch) {
            ret = (char *)s;
        }
    } while(*s++);

    return ret;
}