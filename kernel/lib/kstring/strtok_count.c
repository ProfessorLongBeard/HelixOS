#include <kstring.h>





int strtok_count(const char *s) {
    int count = 0;
    char *ptr = NULL;



    if (!s) {
        return -1;
    }

    ptr = (char *)s;

    while(*ptr) {
        while(*ptr == '/') {
            ptr++;
        }

        if (*ptr == '\0') {
            break;
        }

        count++;

        while(*ptr && *ptr != '/') {
            ptr++;
        }
    }

    return count;
}