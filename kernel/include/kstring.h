#ifndef KSTRING_H
#define KSTRING_H

#include <stdint.h>
#include <stddef.h>



#define abs(x) ((x) > 0 ? (x) : -(x))

#define min(a, b) ({\
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b);\
    _a < _b ? _a : _b;\
    })

#define max(a, b) ({\
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b);\
    _a > _b ? _a : _b;\
    })







void memset(void *dst, char ch, size_t len);
void memcpy(void *dst, void *src, size_t len);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char *strcat(char *dst, const char *src);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t length);
char *strchr(const char *s, char c);
char *strtok(char *str, const char *delim);
char *strdup(const char *s);
char *strrchr(const char *s, char ch);
int memcmp(const void *s1, const void *s2, size_t length);
void char16_to_ascii(char *dst, uint_least16_t *src, size_t length);
size_t strspn(const char *s1, const char *s2);
char *strpbrk(const char *s, const char *brk);
char *strtok_r(char *s, const char *delim, char **save_ptr);

#endif