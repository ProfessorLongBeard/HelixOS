#include <kstdio.h>
#include <kernel.h>
#include <stdbool.h>
#include <framebuffer.h>
#include <pmm.h>
#include <vmm.h>












void helix_init(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        __hcf();
    }

    fb_init();
    pmm_init();

    void *buf1 = pmm_alloc(5);
    
    void *buf2 = pmm_alloc(500);

    void *buf3 = pmm_alloc(75);

    void *buf4 = pmm_alloc(10);

    void *buf5 = pmm_alloc(10000);

    printf("Test!\n");

    __hcf();
}