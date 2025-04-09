#ifndef SLAB_H
#define SLAB_H

#include <stdint.h>
#include <stddef.h>
#include <spinlock.h>




#define SLAB_MIN_SIZE   8
#define SLAB_MAX_SIZE   8192
#define SLAB_COUNT      80


typedef struct slab {
    uint64_t        num_objects;        // Number of slab objects
    uint64_t        free_objects;       // Number of free slab objects
    size_t          object_size;        // Size of object in slab
    void            *data;              // Pointer to raw data
    struct slab     *next;              // Head of freelist
} slab_t;

typedef struct slab_cache {
    uint64_t        num_objects;        // Number of slabs
    slab_t          *slab_list;         // Linked list of slabs in cache
} slab_cache_t;




void slab_init(void);
void *slab_alloc(size_t length);
void slab_free(void *obj, size_t length);
size_t slab_find_ptr_obj_size(void *ptr);

#endif