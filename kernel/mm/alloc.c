#include <kstdio.h>
#include <kstdlib.h>
#include <mm.h>







void *kmalloc(size_t length) {
    void *ptr = NULL;

    ptr = slab_alloc(length);
    assert(ptr != NULL);

    return ptr;
}

void *kcalloc(size_t length) {
    void *ptr = NULL;

    if (length <= PAGE_SIZE) {
        ptr = slab_alloc(length);
        assert(ptr != NULL);

        memset(ptr, 0, length);
        return ptr;
    }

    return NULL;
}

void *krealloc(void *ptr, size_t length) {
    // TODO: Implement slab_alloc() and handle reallocations a bit more properly...
    void *new_ptr = NULL;

    assert(ptr != NULL && length <= PAGE_SIZE);

    new_ptr = slab_alloc(length);
    assert(new_ptr != NULL);

    // Copy data from ptr -> new_ptr
    memcpy(new_ptr, ptr, length);

    // Free old pointer
    slab_free(ptr, length);
    return new_ptr;
}

void kfree(void *ptr) {
    size_t obj_size = 0;

    assert(ptr != NULL);

    obj_size = slab_find_ptr_obj_size(ptr);
    assert(obj_size > 0);

    slab_free(ptr, obj_size);
}