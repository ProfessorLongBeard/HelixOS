#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>





typedef struct freelist_node {
    uint64_t                base;
    uint64_t                end;
    size_t                  size;
    struct freelist_node    *next;
} freelist_t;






void pmm_init(void);
void *pmm_alloc(void);

#endif