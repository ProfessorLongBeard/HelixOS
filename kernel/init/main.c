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
    gic_init();
    timer_init();
    uart_init();

    timer_set(1000);

    printf("Done!\n");

    __hcf();
}