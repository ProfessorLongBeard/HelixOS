#include <kstring.h>







void char16_to_ascii(char *dst, uint_least16_t *src, size_t length) {
    size_t i = 0;

    if (!dst || !src) {
        return;
    }

    for (i = 0; i < length; i++) {
        if (src[i] == 0) {
            break;
        }

        dst[i] = (char)(src[i] & 0xFF);
    }

    dst[i] = '\0';
}