#include <kstring.h>




char *strtok_r(char *s, const char *delim, char **save_ptr) {
   char *token = NULL;

    if (s == NULL) {
        s = *save_ptr;
    }

    if (s == NULL) {
        return NULL;
    }

    s += strspn(s, delim);

    if (*s == '\0') {
        *save_ptr = NULL;
        return NULL;
    }

    token = s;
    s = strpbrk(token, delim);

    if (s == NULL) {
        *save_ptr = NULL;
    } else {
        *s = '\0';
        *save_ptr = s + 1;
    }

    return token;
}