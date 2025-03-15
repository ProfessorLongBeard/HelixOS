#include <pmm.h>
#include <vmm.h>
#include <limine.h>




__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};






uint64_t vmm_get_hhdm_offset(void) {
    struct limine_hhdm_response *hhdm = hhdm_req.response;

    return hhdm->offset;
}