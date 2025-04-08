#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>



#define KHEAP_SIZE  (4 * 1024 * 1024)

#define KSLAB_MIN_SIZE  8
#define KSLAB_MAX_SIZE  4096
#define KSLAB_COUNT     20


typedef struct kslab {
    uint64_t        total_objects;
    uint64_t        used_objects;
    size_t          object_size;
    void            *free_list;
    struct kslab    *next;
} __attribute__((packed)) kslab_t;

typedef struct kslab_cache {
    uint64_t    object_count;
    kslab_t     *slab_list;
} __attribute__((packed)) kslab_cache_t;





void heap_init(void);
void *kmalloc(size_t length);
void kfree(void *ptr);

#endif