#include <kstring.h>
#include <mm/mm.h>





char *strdup(const char *s) {
    char *ptr = NULL;
    size_t s_length = 0;


    if (!s) {
        return NULL;
    }

    s_length = strlen(s) + 1;

    ptr = kmalloc(s_length);

    if (!ptr) {
        return NULL;
    }

    memcpy(ptr, (char *)s, s_length);

    return ptr;
}