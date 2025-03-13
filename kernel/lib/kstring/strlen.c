#include <kstring.h>
#include <stddef.h>





size_t strlen(const char *s) {
    size_t len = 0;

    while(*s != '\0') {
        len++;
        s++;
    }

    return len;
}