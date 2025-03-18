#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>


#define PMM_PAGE_SIZE   4096

#define ALIGN_UP(base, align) ((base + (align - 1)) & ~(align - 1))
#define ALIGN_DOWN(base, align) (base & ~(align - 1))

#define SIZE_TO_PAGES(size, page_size) ((size + (page_size - 1)) / page_size)




typedef struct __page_node {
    uint64_t            base;
    uint64_t            size;
    struct __page_node  *next;
} __attribute__((packed)) page_node_t;

typedef struct __freelist_node {
    uint64_t                base;
    uint64_t                end;
    size_t                  size;
    struct __freelist_node  *next;
} __attribute__((packed)) freelist_t;








void pmm_init(void);
void *pmm_alloc(void);

#endif