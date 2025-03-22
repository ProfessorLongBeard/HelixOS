#include <kstdio.h>
#include <kstdlib.h>
#include <stdbool.h>
#include <mm.h>







static bitmap_t bmp;

static uintptr_t usable_start = 0;
static size_t usable_size = 0;











void pmm_init(void) {
    uint64_t hhdm = mm_get_hhdm_offset();
    uint64_t map_entries = mm_get_num_entries();


    for (uint64_t i = 0; i < map_entries; i++) {
        struct limine_memmap_entry *e = mm_entry_for_each(i);

        if (e->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (usable_size == 0) {
            usable_start = e->base;
        }

        usable_size += e->length;
    }

    bmp.total_pages = usable_size / PAGE_SIZE;
    bmp.bitmap_size = (bmp.total_pages + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
    bmp.bitmap = (uint8_t *)(hhdm + usable_start);
    bmp.bitmap_base = (uintptr_t)bmp.bitmap;

    // Initialize bitmap
    memset(bmp.bitmap, 0, bmp.bitmap_size);

    printf("PMM: Bitmap information:\n");
    printf("PMM: Bitmap region: [0x%lx - 0x%lx] bitmap size: %uKB\n", (uintptr_t)bmp.bitmap - hhdm, (uintptr_t)bmp.bitmap + bmp.bitmap_size - hhdm, bmp.bitmap_size / 1024);
    printf("PMM: Total pages: %u, used pages: %u\n", bmp.total_pages, bmp.used_pages);
}

static bool pmm_is_bit_set(uint64_t idx) {
    return (bmp.bitmap[idx / BITS_PER_BYTE] & (1 << (idx % BITS_PER_BYTE))) != 0;
}

static void pmm_set_bit(uint64_t idx) {
    bmp.bitmap[idx / BITS_PER_BYTE] |= (1 << (idx % BITS_PER_BYTE));
}

static void pmm_clear_bit(uint64_t idx) {
    bmp.bitmap[idx / BITS_PER_BYTE] &= ~(1 << (idx % BITS_PER_BYTE));
}

static uint64_t pmm_find_first_free(void) {
    for (uint64_t i = 0; i < bmp.total_pages; i++) {
        if (!pmm_is_bit_set(i)) {
            return i;
        }
    }

    return -1;
}

void *pmm_alloc(void) {
    void *ptr = NULL;
    uint64_t hhdm = mm_get_hhdm_offset();
    uint64_t idx = pmm_find_first_free();
    uint64_t bmp_end = bmp.bitmap_base + bmp.bitmap_size;

    assert(idx < bmp.total_pages || idx != (uint64_t)-1);
    pmm_set_bit(idx);

    uintptr_t page_addr = bmp_end + (idx * PAGE_SIZE);
    page_addr = ALIGN_UP(page_addr, PAGE_SIZE);

    ptr = (void *)page_addr;

    bmp.used_pages++;

    printf("PMM: Allocated page: 0x%lx\n", (uintptr_t)ptr - hhdm);
    return ptr;
}

void pmm_free(void *ptr) {
    uint64_t hhdm = mm_get_hhdm_offset();
    uintptr_t aligned_ptr = (uintptr_t)ptr;
    uintptr_t orig_ptr = aligned_ptr - (aligned_ptr % PAGE_SIZE);

    uint64_t idx = ((uintptr_t)orig_ptr - bmp.bitmap_base) / PAGE_SIZE;
    assert(idx < bmp.total_pages);

    printf("PMM: Freeing page: 0x%lx", (uintptr_t)ptr - hhdm);

    pmm_clear_bit(idx);
    bmp.used_pages--;
}