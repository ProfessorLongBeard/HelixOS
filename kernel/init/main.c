#include <kstdio.h>
#include <kernel.h>
#include <stdbool.h>
#include <framebuffer.h>
#include <kstdlib.h>
#include <mm.h>





__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);








void helix_init(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        __hcf();
    }

    fb_init();
    mm_init();
    pmm_init();
    vmm_init();

    printf("Test!\n");
    
    __hcf();
}