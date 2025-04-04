#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <spinlock.h>



#define BITS_PER_BYTE   8



typedef struct {
    spinlock_t  s;

    uint64_t    phys_start;
    uint64_t    phys_end;
    size_t      phys_size;

    uint64_t    bitmap_base;
    uint64_t    bitmap_end;
    size_t      bitmap_size;

    size_t      total_pages;
    size_t      used_pages;
    size_t      reserved_pages;

    uint8_t     *bitmap;
} bitmap_t;


void pmm_init(struct limine_memmap_entry **mm, uint64_t mm_count);
void *pmm_alloc(void);
void pmm_free(void *ptr);

#endif