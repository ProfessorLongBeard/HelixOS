#ifndef KSTRING_H
#define KSTRING_H

#include <stddef.h>







void memset(void *dst, char ch, size_t len);
void memcpy(void *dst, void *src, size_t len);
size_t strlen(const char *s);

#endif