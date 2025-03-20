#include <kstdio.h>
#include <kernel.h>
#include <stdbool.h>
#include <framebuffer.h>
#include <kstdlib.h>
#include <mm.h>













void helix_init(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        __hcf();
    }

    fb_init();
    mm_init();
    pmm_init();

    void *ptr = pmm_alloc();
    printf("ptr = 0x%lx\n", (uint64_t)ptr);

    __hcf();
}