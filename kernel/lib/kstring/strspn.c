#include <kstring.h>




size_t strspn(const char *s1, const char *s2) {
    size_t count = 0;


    while(*s1) {
        int found = 0;
        const char *tmp = s2;

        while(*tmp) {
            if (*s1 == *tmp) {
                found = 1;
                break;
            }

            tmp++;
        }

        if (found) {
            count++;
            s1++;
        } else {
            break;
        }
    }

    return count;
}