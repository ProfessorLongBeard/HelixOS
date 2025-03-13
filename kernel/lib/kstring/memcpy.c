#include <kstring.h>
#include <stdint.h>





void memcpy(void *dst, void *src, size_t len) {
    uint8_t *d = (uint8_t *)dst;
    uint8_t *s = (uint8_t *)src;


    for (uint64_t i = 0; i < len; i++) {
        d[i] = s[i];
    }
}