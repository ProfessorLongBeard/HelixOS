#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>




typedef struct __freelist_node {
    uint64_t                base_addr;
    size_t                  length;
    struct __freelist_node  *next;
} freelist_t;







void pmm_init(void);
void *pmm_alloc(size_t len);

#endif