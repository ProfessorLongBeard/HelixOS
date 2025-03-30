#include <kstdio.h>
#include <kstdlib.h>
#include <stdbool.h>
#include <mm.h>
#include <kernel.h>







static bitmap_t bmp;








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

void pmm_init(void) {
    uint64_t hhdm = mm_get_hhdm_offset();
    uint64_t map_entries = mm_get_num_entries();

    spinlock_init(&bmp.s);

    for (uint64_t i = 0; i < map_entries; i++) {
        struct limine_memmap_entry *e = mm_entry_for_each(i);

        if (e->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (bmp.usable_size == 0) {
            // Get largest usable regions (TODO: check and verify this)
            bmp.usable_start = e->base;
            bmp.usable_size = e->length;
        }
    }

    bmp.usable_end = bmp.usable_start + bmp.usable_size;
    bmp.total_pages = bmp.usable_size / PAGE_SIZE;
    bmp.bitmap_size = (bmp.total_pages + BITS_PER_BYTE - 1) / BITS_PER_BYTE;

    bmp.bitmap = (uint8_t *)(hhdm + bmp.usable_start);
    bmp.bitmap_base = (uint64_t)bmp.bitmap;

    // Intiialize bitmap
    memset((uint8_t *)bmp.bitmap, 0, PAGE_SIZE);

    // Bitmap itself should be marked as in use
    bmp.reserved_pages = SIZE_TO_PAGES(bmp.bitmap_size, PAGE_SIZE);

    for (size_t i = 0; i < bmp.reserved_pages; i++) {
        // Mark bitmap pages as in use
        pmm_set_bit(i);
        bmp.used_pages++;
    }

    printf("PMM: Bitmap information:\n");
    printf("PMM: Usable region: [0x%lx - 0x%lx] region size: %uMB\n", bmp.usable_start, bmp.usable_end, bmp.usable_size / (1024 * 1024));
    printf("PMM: Bitmap region: [0x%lx - 0x%lx] bitmap size: %uKB\n", (uint64_t)bmp.bitmap - hhdm, (uint64_t)bmp.bitmap + bmp.bitmap_size - hhdm, bmp.bitmap_size / 1024);
    printf("PMM: Total pages: %llu, used pages: %llu, reserved pages: %llu\n", bmp.total_pages, bmp.used_pages, bmp.reserved_pages);
}

uint64_t pmm_get_bitmap_base(void) {
    return bmp.bitmap_base;
}

size_t pmm_get_bitmap_size(void) {
    return bmp.bitmap_size;
}

void *pmm_alloc(void) {
    void *ptr = NULL;
    uint64_t hhdm = mm_get_hhdm_offset();
    uint64_t idx = pmm_find_first_free();
    uint64_t bmp_end = bmp.bitmap_base + bmp.bitmap_size;

    spinlock_acquire(&bmp.s);

    assert(idx < bmp.total_pages || idx != (uint64_t)-1);

    // Ensure next allocated frame is aligned on PAGE_SIZE boundaries
    uint64_t page_addr = bmp_end + (idx * PAGE_SIZE);
    page_addr = ALIGN_UP(page_addr, PAGE_SIZE);

    ptr = (void *)page_addr;

    pmm_set_bit(idx);
    bmp.used_pages++;

    spinlock_release(&bmp.s);

    return ptr;
}

void *pmm_allocz(void) {
    void *ptr = pmm_alloc();
    assert(ptr != NULL);

    memset(ptr, 0, PAGE_SIZE);
    return ptr;
}

void pmm_free(void *ptr) {
    uint64_t hhdm = mm_get_hhdm_offset();
    uint64_t aligned_ptr = (uint64_t)ptr;
    uint64_t orig_ptr = 0;

    spinlock_acquire(&bmp.s);

    orig_ptr = aligned_ptr - (aligned_ptr % PAGE_SIZE);

    uint64_t idx = ((uint64_t)orig_ptr - bmp.bitmap_base) / PAGE_SIZE;
    assert(idx <= bmp.total_pages);

    pmm_clear_bit(idx);
    bmp.used_pages--;

    spinlock_release(&bmp.s);
}