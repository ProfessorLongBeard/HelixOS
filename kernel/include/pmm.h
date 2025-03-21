#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>



#define BITS_PER_BYTE   8

#define PMM_BIT_INDEX(x) (x / BITS_PER_BYTE)
#define PMM_BIT_OFFSET(x) (x % BITS_PER_BYTE)



typedef struct {
    uintptr_t   bitmap_base;
    size_t      bitmap_size;
    size_t      total_pages;
    size_t      used_pages;
    uint8_t     *bitmap;
} bitmap_t;


void pmm_init(void);
void *pmm_alloc(void);
void pmm_free(void *ptr);

#endif