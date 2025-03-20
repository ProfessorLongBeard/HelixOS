#include <mm.h>
#include <kstdlib.h>





__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request mm = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

struct limine_memmap_response *m = NULL;




void mm_init(void) {
    if (!m) {
        m = mm.response;
    }
}

uint32_t mm_get_num_entries(void) {
    return m->entry_count;
}

struct limine_memmap_entry *mm_entry_for_each(uint32_t idx) {
    struct limine_memmap_entry *e = NULL;


    if (idx < 0 || idx > m->entry_count) {
        printf("Invalid memory map entry index: %u\n", idx);
    }

    e = m->entries[idx];
    assert(e != NULL);

    return e;
}