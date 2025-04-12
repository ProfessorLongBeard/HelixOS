#include <kstdio.h>
#include <kstdlib.h>
#include <mm.h>







void *kmalloc(size_t length) {
    void *ptr = NULL;
    size_t page_count = 0;


    if (length <= SLAB_MAX_SIZE) {
        ptr = slab_alloc(length);
        assert(ptr != NULL);

        memset(ptr, 0, length);
        return ptr;
    }

    page_count = SIZE_TO_PAGES(length, PAGE_SIZE);

    // TODO: Use better allocation method(s) for larger sizes
    ptr = pmm_alloc_pages(page_count);
    assert(ptr != NULL);

    memset(ptr, 0, length);

    return ptr;
}

void kfree(void *ptr, size_t length) {
    assert(ptr != NULL);

    slab_free(ptr, length);
}