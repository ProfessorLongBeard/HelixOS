#include <kstdio.h>






void puts(const char *s) {
    while(*s != '\0') {
        putc(*s);
        s++;
    }
}