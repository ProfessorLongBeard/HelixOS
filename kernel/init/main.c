#include <kstdio.h>
#include <kstring.h>
#include <kernel.h>
#include <arch.h>
#include <framebuffer.h>
#include <stdbool.h>










void helix_init(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        __hcf();
    }

    fb_init();

    __hcf();
}