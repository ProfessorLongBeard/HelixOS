#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>




void *kmalloc(size_t length);
void *kcalloc(size_t length);
void *krealloc(void *ptr, size_t length);
void kfree(void *ptr);

#endif