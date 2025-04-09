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

void pmm_init(struct limine_memmap_response *m) {
    struct limine_memmap_entry *e = NULL;

    spinlock_init(&bmp.s);


    for (uint64_t i = 0; i < m->entry_count; i++) {
        e = m->entries[i];

        if (e->type != LIMINE_MEMMAP_USABLE) {
            // We only want usable physical memory
            continue;
        }

        if (bmp.phys_start == 0) {
            bmp.phys_start = e->base;
        }

        bmp.phys_size += e->length;
    }

    bmp.phys_end = bmp.phys_start + bmp.phys_size;

    bmp.total_pages = bmp.phys_size / PAGE_SIZE;

    bmp.bitmap_size = (bmp.total_pages + BITS_PER_BYTE - 1) / BITS_PER_BYTE;    
    bmp.bitmap = (uint8_t *)PHYS_TO_VIRT((uint64_t)bmp.phys_start);
    bmp.bitmap_base = (uint64_t)bmp.bitmap;
    bmp.bitmap_end  = bmp.bitmap_base + bmp.bitmap_size;

    bmp.reserved_pages = SIZE_TO_PAGES(bmp.bitmap_size, PAGE_SIZE);

    // Initialize bitmap
    memset(bmp.bitmap, 0, bmp.bitmap_size);

    for (size_t i = 0; i < bmp.reserved_pages; i++) {
        pmm_set_bit(i);
        bmp.used_pages++;
    }

    printf("PMM: Available memory region: 0x%lx - 0x%lx length: %lluGB\n", bmp.phys_start, bmp.phys_end, bmp.phys_size / 1024 / 1024 / 1024);    
    printf("PMM: Bitmap: 0x%lx - 0x%lx length: %lluKB\n", VIRT_TO_PHYS(bmp.bitmap_base), VIRT_TO_PHYS(bmp.bitmap_end), bmp.bitmap_size / 1024);
    printf("PMM: Total pages: %llu used pages: %llu reserved pages: %llu\n", bmp.total_pages, bmp.used_pages, bmp.reserved_pages);
}

uint64_t pmm_get_bitmap_base(void) {
    return bmp.bitmap_base;
}

size_t pmm_get_bitmap_size(void) {
    return bmp.bitmap_size;
}

void *pmm_alloc(void) {
    void *ptr = NULL;
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

void pmm_free(void *ptr) {
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