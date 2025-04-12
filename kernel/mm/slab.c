#include <kstdio.h>
#include <kstdlib.h>
#include <mm.h>
#include <spinlock.h>





static spinlock_t s;
static slab_cache_t slab_cache[SLAB_COUNT];





static slab_cache_t *slab_cache_for_each(int slab_order) {
    slab_cache_t *cache = NULL;

    assert(slab_order >= 0 || slab_order <= SLAB_COUNT);

    cache = &slab_cache[slab_order];

    return cache;
}

static int slab_get_order(size_t length) {
    int order = 0;
    size_t s = SLAB_MIN_SIZE;

    while(s <= length && order <= SLAB_COUNT - 1) {
        s <<= 1;
        order++;
    }

    return order;
}

static size_t slab_get_size(size_t length) {
    size_t slab_size = SLAB_MIN_SIZE;

    // Get first-fit/best-git length from a given alloction request

    while(slab_size <= length) {
        slab_size <<= 1;
    }

    return slab_size;
}

void slab_init(void) {
    void *ptr = NULL;
    slab_t *slab = NULL;
    size_t slab_size = SLAB_MIN_SIZE;
    size_t page_count = 0, slab_bytes = 0, usable = 0;



    spinlock_init(&s);

    for (int i = 0; i < SLAB_COUNT && slab_size <= SLAB_MAX_SIZE; i++) {
        slab_cache_t *cache = slab_cache_for_each(i);

        page_count = SIZE_TO_PAGES(slab_size, PAGE_SIZE);

        slab = (page_count == 1) ? pmm_alloc() : pmm_alloc_pages(page_count);
        assert(slab != NULL);

        uintptr_t base = (uintptr_t)slab + sizeof *slab;
        uintptr_t base_aligned = (base + slab_size - 1) & ~(slab_size - 1);

        slab_bytes = page_count * PAGE_SIZE;
        usable = slab_bytes - (base_aligned - (uintptr_t)slab);

        slab->total_objects = usable / slab_size;
        slab->free_objects = slab->total_objects;
        slab->used_objects = 0;
        slab->object_size = slab_size;
        slab->next = NULL;

        ptr = (void *)base_aligned;
        
        for (uint64_t i = 0; i < slab->total_objects; i++) {
            *(void **)ptr = slab->free_list;
            slab->free_list = ptr;
            ptr = (void *)((uint8_t *)ptr + slab->object_size);
        }

        slab->next = cache->slab_list;
        cache->object_count = slab->total_objects;
        cache->slab_list = slab;

        slab_size <<= 1;
    }

    printf("SLAB: Initialized!\n");
}

static slab_t *slab_create_new(slab_t *old_slab, size_t length) {
    void *ptr = NULL;
    size_t page_count = 0, slab_bytes = 0, usable = 0;
    slab_t *new_slab = NULL;
    size_t slab_size = 0;



    assert(length <= SLAB_MAX_SIZE);

    page_count = SIZE_TO_PAGES(length, PAGE_SIZE);

    // Get next power of 2 length for allocation request
    slab_size = slab_get_size(length);

    new_slab = (page_count == 1) ? pmm_alloc() : pmm_alloc_pages(page_count);
    assert(new_slab != NULL);

    uintptr_t base = (uintptr_t)new_slab + sizeof *new_slab;
    uintptr_t base_aligned = (base + slab_size- 1) & ~(slab_size - 1);

    slab_bytes = page_count * PAGE_SIZE;
    usable = slab_bytes - (base_aligned - (uintptr_t)new_slab);

    new_slab->total_objects = usable / slab_size;
    new_slab->free_objects = new_slab->total_objects;
    new_slab->used_objects = 0;
    new_slab->object_size = slab_size;
    new_slab->next = NULL;

    ptr = (void *)base_aligned;
    
    for (uint64_t i = 0; i < new_slab->total_objects; i++) {
        *(void **)ptr = new_slab->free_list;
        new_slab->free_list = ptr;
        ptr = (void *)((uint8_t *)ptr + new_slab->object_size);
    }

    return new_slab;
}

static void *slab_get_object(slab_t *slab, size_t length) {
    void *ptr = NULL;

    assert(slab != NULL);
    assert(length <= SLAB_MAX_SIZE);

    if (slab->free_objects == 0 || slab->used_objects == slab->total_objects) {
        printf("SLAB: No available object in slab: 0x%lx\n", (uintptr_t)slab);
        return NULL;
    }

    assert(slab->free_list != NULL);
    ptr = (void *)slab->free_list;
    slab->free_list = *(void **)ptr;

    slab->free_objects--;
    slab->used_objects++;
    
    return ptr;
}

void *slab_alloc(size_t length) {
    void *ptr = NULL;
    slab_t *slab = NULL;
    slab_cache_t *cache = NULL;
    int slab_order = 0;



    assert(length <= SLAB_MAX_SIZE);

    spinlock_acquire(&s);

    if (length < SLAB_MIN_SIZE) {
        length = SLAB_MIN_SIZE;
    }

    slab_order = slab_get_order(length);
    assert(slab_order <= SLAB_COUNT);

    cache = slab_cache_for_each(slab_order);
    assert(cache != NULL);
    
    slab = cache->slab_list;
    assert(slab != NULL);

    while(slab->next != NULL) {
        if (slab->free_objects == 0 || slab->used_objects == slab->total_objects) {
            slab = slab->next;
        }
    }

    if (slab->free_objects == 0 || slab->used_objects == slab->total_objects) {
        slab_t *new_slab = slab_create_new(slab, length);
        assert(new_slab != NULL);

        // Set new allocated slab as head in list
        slab->next = new_slab;
        slab = slab->next;
    }

    if (slab->used_objects < slab->total_objects && slab->object_size >= length) {
        ptr = slab_get_object(slab, slab->object_size);
        assert(ptr != NULL);

        spinlock_release(&s);
        return ptr;
    }

    spinlock_release(&s);
    return NULL;
}

void slab_free(void *ptr, size_t length) {
    slab_t *slab = NULL;
    slab_cache_t *cache = NULL;
    int slab_order = 0;
    size_t slab_size = 0;
    uintptr_t obj_start = (uintptr_t)ptr;


    assert(ptr != NULL);

    spinlock_acquire(&s);

    slab_order = slab_get_order(length);

    cache = slab_cache_for_each(slab_order);
    assert(cache != NULL);

    slab_size = slab_get_size(length);

    slab = cache->slab_list;
    assert(slab != NULL);

    while(slab->next != NULL && slab->free_objects == 0) {
        slab = slab->next;
    }

    uintptr_t freelist_start = (uintptr_t)slab + sizeof *slab;
    uintptr_t freelist_end = freelist_start + (slab->total_objects * slab->object_size);

    if (obj_start >= freelist_start && obj_start <= freelist_end) {
        *(void **)obj_start = slab->free_list;
        slab->free_list = (void *)obj_start;

        slab->free_objects++;
        slab->used_objects--;
    }

    spinlock_release(&s);
}