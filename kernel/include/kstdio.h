#ifndef KSTDIO_H
#define KSTDIO_H

#include <printf.h>



int print(const char *fmt, ...);

int printf(const char *fmt, ...);
int sprintf(char *s, const char *fmt, ...);
int snprintf(char *s, size_t count, const char *fmt, ...);

void putc(char ch);
void puts(const char *s);

void putchar_(char ch);

#endif