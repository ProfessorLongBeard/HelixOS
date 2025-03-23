#include <mm.h>
#include <kstdlib.h>





__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request mm = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

struct limine_memmap_response *m = NULL;
struct limine_hhdm_response *hhdm = NULL;








void mm_init(void) {
    if (!m) {
        m = mm.response;
    }

    if (!hhdm) {
        hhdm = hhdm_req.response;
    }
}

uint32_t mm_get_num_entries(void) {
    return m->entry_count;
}

struct limine_memmap_entry *mm_entry_for_each(uint32_t idx) {
    struct limine_memmap_entry *e = NULL;


    if (idx > m->entry_count) {
        printf("Invalid memory map entry index: %u\n", idx);
    }

    e = m->entries[idx];
    assert(e != NULL);

    return e;
}

uint64_t mm_get_hhdm_offset(void) {
    return hhdm->offset;
}

struct limine_memmap_entry *mm_get_entry_by_type(uint32_t type) {
    struct limine_memmap_entry *entry = NULL;
    uint32_t num_entries = mm_get_num_entries();


    for (uint32_t i = 0; i < num_entries; i++) {
        struct limine_memmap_entry *e = mm_entry_for_each(i);

        if (e->type == type) {
            entry = e;
            break;
        }
    }

    return entry;
}