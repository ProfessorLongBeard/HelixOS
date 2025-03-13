#include <kstring.h>
#include <kernel.h>
#include <limine.h>
#include <arch.h>
#include <stdbool.h>
#include <devices/pl011.h>




__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};








void helix_init(void) {
    struct flanterm_context *ft = NULL;

    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        __hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    __hcf();
}