#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>



#define BITS_PER_BYTE   8



typedef struct {
    uint64_t    bitmap_base;
    size_t      bitmap_size;
    size_t      total_pages;
    size_t      used_pages;
    uint8_t     *bitmap;
} bitmap_t;


void pmm_init(void);
void *pmm_alloc(void);
void pmm_free(void *ptr);
uint64_t pmm_get_bitmap_base(void);
size_t pmm_get_bitmap_size(void);

#endif