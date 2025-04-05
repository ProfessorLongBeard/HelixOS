#include <kstdio.h>
#include <kstdlib.h>
#include <stdbool.h>
#include <pmm.h>
#include <vmm.h>
#include <kernel.h>




pmm_t pmm;










void pmm_init(struct limine_memmap_entry **mm, uint64_t mm_count) {
    uint64_t highest_addr = 0;

    assert(mm != NULL && mm_count > 0);

    spinlock_init(&pmm.s);

    pmm.hhdm = VMM_VIRT_BASE;

    for (uint64_t i = 0; i < mm_count; i++) {
        struct limine_memmap_entry *e = mm[i];

        uint64_t base = e->base;
        uint64_t length = e->length;

        if (e->type == LIMINE_MEMMAP_USABLE || e->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE || e->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            uint64_t top = base + length;

            if (top > highest_addr) {
                highest_addr = top;
            }
        }
    }

    pmm.free_pages = (highest_addr + PAGE_SIZE - 1) / PAGE_SIZE;
    pmm.bitmap_size = ALIGN_UP(pmm.free_pages / 8, PAGE_SIZE);

    for (uint64_t i = 0; i < mm_count; i++) {
        struct limine_memmap_entry *e = mm[i];

        if (e->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        pmm.bitmap = (uint8_t *)(pmm.hhdm + e->base);

        memset((uint8_t *)pmm.bitmap, 0xFF, pmm.bitmap_size);

        e->base += pmm.bitmap_size;
        e->length -= pmm.bitmap_size;
        break;
    }

    for (uint64_t i = 0; i < mm_count; i++) {
        struct limine_memmap_entry *e = mm[i];

        uint32_t type = e->type;
        uint64_t base = e->base;
        uint64_t length = e->length;

        if (type == LIMINE_MEMMAP_USABLE) {
            for (uint64_t j = 0; j < length; j += PAGE_SIZE) {
                pmm.free_pages++;
                pmm.bitmap[(base + j) / PAGE_SIZE / 8] &= ~(1UL << ((base + j) % 8));
            }
        }
    }

    pmm.bitmap_base = (uint64_t)pmm.bitmap;
    pmm.bitmap_end = pmm.bitmap_base + pmm.bitmap_size;

    pmm.used_pages += SIZE_TO_PAGES(pmm.bitmap_size, PAGE_SIZE);
    pmm.free_pages -= SIZE_TO_PAGES(pmm.bitmap_size, PAGE_SIZE);

    printf("PMM: Bitmap region: [0x%lx - 0x%lx] length: %lluKB\n", pmm.bitmap_base, pmm.bitmap_end, pmm.bitmap_size / 1024);
    printf("PMM: Free pages: %llu used pages: %llu\n", pmm.free_pages, pmm.used_pages);
}

static void *pmm_internal_alloc(uint64_t page_count) {
    uint64_t last_used = pmm.last_used_idx;
    uint64_t ret = 0;

    spinlock_acquire(&pmm.s);

    for (uint64_t i = pmm.last_used_idx; i < pmm.free_pages; i++) {
        if (!(pmm.bitmap[i / 8] & (1UL << (i % 8)))) {
            pmm.last_used_idx++;
            ret++;

            if (ret == page_count) {
                uint64_t page = pmm.last_used_idx - page_count;

                for (uint64_t j = page; j < pmm.last_used_idx; j++) {
                    pmm.bitmap[j / 8] |= (1UL << (j % 8));
                }

                spinlock_release(&pmm.s);
                return (void *)(page * PAGE_SIZE);
            }
        } else {
            pmm.last_used_idx++;
            ret = 0;
        }
    }

    pmm.last_used_idx = 0;

    for (uint64_t i = pmm.last_used_idx; i < last_used; i++) {
        if (!(pmm.bitmap[i / 8] & (1UL << (i % 8)))) {
            pmm.last_used_idx++;
            ret++;

            if (ret == page_count) {
                uint64_t page = pmm.last_used_idx - page_count;

                for (uint64_t j = page; j < pmm.last_used_idx; j++) {
                    pmm.bitmap[j / 8] |= (1UL << (j % 8));
                }

                spinlock_release(&pmm.s);
                return (void *)(page * PAGE_SIZE);
            }
        } else {
            pmm.last_used_idx++;
            ret = 0;
        }
    }

    pmm.used_pages += page_count;
    pmm.free_pages -= page_count;

    spinlock_release(&pmm.s);
    return NULL;
}

void *pmm_alloc(uint64_t page_count) {
    void *ptr = NULL;

    assert(page_count < pmm.free_pages);

    ptr = pmm_internal_alloc(page_count);
    if (!ptr) {
        printf("PMM: Out of memory!\n");
    }

    memset((pmm.hhdm + ptr), 0, page_count * PAGE_SIZE);
    return ptr;
}

void pmm_free(void *ptr, uint64_t page_count) {
    uint64_t page = (uint64_t)ptr / PAGE_SIZE;

    spinlock_acquire(&pmm.s);

    for (uint64_t i = page; i < page + page_count; i++) {
        pmm.bitmap[i / 8] &= ~(1UL << (i % 8));
    }

    pmm.free_pages += page_count;
    pmm.used_pages -= page_count;

    spinlock_release(&pmm.s);
}