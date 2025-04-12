#include <kstdio.h>
#include <kernel.h>
#include <stdbool.h>
#include <framebuffer.h>
#include <kstdlib.h>
#include <mm.h>
#include <arch.h>
#include <devices/timer.h>
#include <devices/gicv3.h>
#include <devices/pl011.h>
#include <devices/virtio/virtio.h>
#include <devices/virtio/virtio_blk.h>








__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request mm = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_kernel_address_request kern_addr_req = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 3
};






void helix_init(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        __hcf();
    }

    struct limine_framebuffer_response *f = fb_req.response;
    struct limine_memmap_response *m = mm.response;
    struct limine_kernel_address_response *kr = kern_addr_req.response;


    if (f->framebuffer_count >= 1) {
        struct limine_framebuffer *fb = f->framebuffers[0];
        fb_init(fb);
    }

    pmm_init(m);
    vmm_init(kr->physical_base, kr->virtual_base, m);
    slab_init();
    gic_init();
    timer_init();
    uart_init();
    virtio_init();

    __hcf();
}