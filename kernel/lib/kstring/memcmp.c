#include <kstring.h>
#include <stdint.h>






int memcmp(const void *s1, const void *s2, size_t length) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;


    for (size_t i = 0; i < length; i++) {
        if (p1[i] != p2[i]) {
            return (p1[i] < p2[i]) ? -1 : 1;
        }
    }

    return 0;
}