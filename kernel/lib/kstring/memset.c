#include <kstring.h>
#include <stdint.h>







void memset(void *dst, char ch, size_t len) {
    uint8_t *d = (uint8_t *)dst;

    for (uint64_t i = 0; i < len; i++) {
        d[i] = ch;
    }
}