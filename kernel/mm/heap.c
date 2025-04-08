#include <kstdio.h>
#include <kstdlib.h>
#include <kstring.h>
#include <vmm.h>
#include <heap.h>
#include <spinlock.h>






static spinlock_t s;
static size_t kheap_max_size = KHEAP_SIZE;
static size_t kheap_used_size = 0;


__attribute__((section(".heap"))) static uint8_t heap[KHEAP_SIZE] = {0};

static kslab_cache_t kslab_cache[KSLAB_COUNT];







static size_t kslab_get_order(size_t length) {
    size_t order = 0;

    while(length > KSLAB_MIN_SIZE) {
        length >>= 1;
        order++;
    }

    assert(order <= KSLAB_COUNT);

    return order;
}

static kslab_cache_t *kslab_cache_for_each(size_t slab_order) {
    kslab_cache_t *cache = NULL;

    assert(slab_order >= 1 || slab_order <= KSLAB_COUNT);

    cache = (kslab_cache_t *)&kslab_cache[slab_order];
    assert(cache != NULL);

    return cache;
}

static void *kheap_alloc(size_t length) {
    void *ptr = NULL;

    assert(length <= kheap_max_size && length >= 1);
    assert(kheap_used_size <= kheap_max_size);

    ptr = (void *)(heap + kheap_used_size);
    assert(ptr != NULL);

    kheap_used_size += length;

    return ptr;
}

static void kheap_free(void *ptr, size_t length) {
    void *p = NULL;

    assert(ptr != NULL);

    assert(length <= kheap_max_size);
    assert(kheap_used_size >= 1 || kheap_used_size <= kheap_max_size);

    p = (void *)(heap + kheap_used_size - length);
    assert(p != NULL);

    // TODO: Make sure we don't overwrite other data doing this!
    memset(p, 0, length);

    kheap_used_size -= length;
}

static void *kheap_alloc_aligned(size_t length, uint64_t align) {
    void *ptr = NULL;
    size_t length_aligned = 0;

    assert(length <= kheap_max_size && length >= 1);
    assert(kheap_used_size <= kheap_max_size);

    length_aligned = ALIGN_UP(length, align);

    if (length_aligned + length > kheap_max_size) {
        return NULL;
    }

    ptr = (void *)&heap[length_aligned];
    assert(ptr != NULL);

    return ptr;
}

void heap_init(void) {
    void *ptr = NULL;
    size_t length = KSLAB_MIN_SIZE;



    spinlock_init(&s);

    for (size_t i = 0; i < KSLAB_COUNT && length <= KSLAB_MAX_SIZE; i++) {
        kslab_cache_t *cache = kslab_cache_for_each(i);
        assert(cache != NULL);

        kslab_t *slab = kheap_alloc(length);
        assert(slab != NULL);

        slab->total_objects = PAGE_SIZE / length;
        slab->object_size = length;
        slab->used_objects = 0;
        slab->free_list = (void *)(slab + 1);
        ptr = (void *)(slab + 1);

        for (size_t i = 0; i < slab->total_objects - 1; i++) {
            *(void **)ptr = slab->free_list;
            slab->free_list = ptr;
            ptr = (void *)((uint8_t *)ptr + slab->object_size);
        }

        slab->next = cache->slab_list;
        cache->object_count = slab->total_objects;
        cache->slab_list = slab;

        length <<= 1;
    }

    printf("KSLAB: Initialized!\n");
}

static kslab_t *kslab_create_new(size_t length) {
    void *ptr = NULL;
    kslab_t *slab = NULL;
    kslab_cache_t *cache = NULL;
    size_t order_count = kslab_get_order(length);


    cache = kslab_cache_for_each(order_count);
    assert(cache != NULL);

    slab = kheap_alloc(length);
    assert(slab != NULL);

    slab->total_objects = PAGE_SIZE / length;
    slab->object_size = length;
    slab->used_objects = 0;
    slab->free_list = (void *)(slab + 1);
    ptr = (void *)(slab + 1);

    for (size_t i = 0; i < slab->total_objects; i++) {
        *(void **)ptr = slab->free_list;
        slab->free_list = ptr;
        ptr = (void *)((uint8_t *)ptr + slab->object_size);
    }

    slab->next = cache->slab_list;
    cache->object_count = slab->total_objects;
    cache->slab_list = slab;

    return slab;
}

static void *kslab_get_object(kslab_t *slab, size_t length) {
    void *ptr = NULL;
    kslab_t *curr = slab;
    
    assert(curr != NULL);
    
    if (curr->free_list == NULL) {
        return NULL;
    }

    ptr = curr->free_list;
    assert(ptr != NULL);

    curr->free_list = *(void **)ptr;
    curr->used_objects++;

    return ptr;
}

static kslab_t *kslab_find_first_free(size_t length) {
    size_t slab_order = kslab_get_order(length);
    kslab_t *slab = NULL;
    kslab_cache_t *cache = NULL;


    cache = kslab_cache_for_each(slab_order);
    assert(cache != NULL);

    slab = cache->slab_list;

    while(slab->next != NULL) {
        if (slab->used_objects < slab->total_objects && slab->object_size == length) {
            return slab;
        }

        slab = slab->next;
    }

    return NULL;
}

static void kslab_link_to_cache(kslab_t *slab) {
    kslab_cache_t *cache = NULL;
    size_t order = 0;


    assert(slab != NULL);

    order = kslab_get_order(slab->object_size);
    cache = kslab_cache_for_each(order);
    assert(cache != NULL);

    if (cache->slab_list == NULL) {
        cache->slab_list = slab;
    } else {
        slab->next = cache->slab_list;
        cache->slab_list = slab;
    }

    cache->object_count++;
}

size_t kslab_find_ptr_object_size(void *ptr) {
    size_t order = KSLAB_MIN_SIZE;
    kslab_t *slab = NULL;
    kslab_cache_t *cache = NULL;



    assert(ptr != NULL);

    for (size_t i = 0; i < KSLAB_COUNT && order <= KSLAB_MAX_SIZE; i++) {
        cache = kslab_cache_for_each(i);
        assert(cache != NULL);

        slab = cache->slab_list;

        while(slab != NULL) {
            size_t slab_size = slab->total_objects * slab->object_size;
            uintptr_t freelist_start = (uintptr_t)(slab + 1);
            uintptr_t freelist_end = freelist_start + slab_size;

            if ((uintptr_t)ptr >= freelist_start && (uintptr_t)ptr < freelist_end) {
                for (uintptr_t obj = freelist_start; obj < freelist_end; obj += slab->object_size) {
                    if ((uintptr_t)ptr == obj) {
                        return slab->object_size;
                    }
                }
            }

            slab = slab->next;
        }
    }

    return 0;
}

void *kmalloc(size_t length) {
    void *ptr = NULL;
    size_t order = kslab_get_order(length);
    kslab_t *slab = NULL;
    kslab_cache_t *cache = NULL;


    spinlock_acquire(&s);

    cache = kslab_cache_for_each(order);
    assert(cache != NULL);

    if (cache->slab_list == NULL) {
        slab = kslab_create_new(length);
        cache->slab_list = slab;
        cache->object_count = slab->total_objects;
    } else {
        slab = cache->slab_list;

        if (slab->used_objects < slab->total_objects) {
            ptr = kslab_get_object(slab, length);
            assert(ptr != NULL);

            spinlock_release(&s);
            return ptr;
        } else {
            slab = kslab_create_new(length);
            assert(slab != NULL);

            cache->slab_list = slab;
            cache->object_count = slab->total_objects;
        }
    }

    ptr = kslab_get_object(slab, length);
    assert(ptr != NULL);

    spinlock_release(&s);
    return ptr;
}

void kfree(void *ptr) {
    kslab_t *slab = NULL;
    kslab_cache_t *cache = NULL;
    size_t order = 0;



    assert(ptr != NULL);

    spinlock_acquire(&s);

    order = kslab_find_ptr_object_size(ptr);
    cache = kslab_cache_for_each(order);
    assert(cache != NULL);

    slab = cache->slab_list;
    assert(slab != NULL);

    size_t slab_size = slab->total_objects * slab->object_size;
    uintptr_t freelist_start = (uintptr_t)(slab + 1);
    uintptr_t freelist_end = freelist_start + slab_size;

    uintptr_t object_start = (uintptr_t)ptr;
    uintptr_t object_end = object_start + slab->object_size;

    size_t object_idx = (object_start - freelist_start) / slab->object_size;
    uintptr_t base = freelist_start + object_idx * slab->object_size;

    if (object_start != base) {
        printf("KSLAB: Failed to locate slab object: 0x%lx\n", (uintptr_t)ptr);
        
        spinlock_release(&s);
        return;
    }

    if (slab->free_list != NULL) {
        *(void **)ptr = slab->free_list;
        slab->used_objects--;
    }

    slab->free_list = ptr;
    slab->used_objects--;

    if (slab->used_objects == 0) {
        kheap_free(slab, sizeof(kslab_t));
    }

    spinlock_release(&s);
}