#include <kstdio.h>
#include <kstdlib.h>
#include <mm/mm.h>
#include <arch.h>
#include <spinlock.h>
#include <devices/virtio/virtio.h>
#include <devices/virtio/virtio_blk.h>






static spinlock_t s;
static vmm_mdl_t *vq_mdl = NULL;
virtio_queue_t *vq = NULL;







size_t virtio_pad_bytes(size_t length) {
    size_t pad = length % 512;
    size_t new_length = length;

    if (pad != 0) {
        new_length += (512 - pad);
    }

    return new_length;
}

void virtio_blk_init(void) {
    spinlock_init(&s);

    vq = (virtio_queue_t *)(uintptr_t)virtio_blk_queue_init(VIRTIO_QUEUE_SIZE);
    assert(vq != NULL);
}

uint32_t virtio_blk_read(uint8_t *buf, uint64_t sector, size_t length) {
    uint8_t *status = NULL;
    virtio_blk_req_t *req = NULL;
    virtio_desc_t *desc_hdr = NULL, *desc_data = NULL, *desc_status = NULL;
    uint32_t desc_hdr_index = 0, desc_data_index = 0, desc_status_index = 0;



    assert(buf != NULL);

    spinlock_acquire(&s);

    // Pad to 512 bytes
    length = virtio_pad_bytes(length);

    req = kmalloc(sizeof(virtio_blk_req_t));
    assert(req != NULL);

    // Read command
    req->type = VIRTIO_BLK_T_IN;
    req->sector = sector;
    req->reserved = 0;

    status = kmalloc(1);
    assert(status != NULL);

    *status = 0xFF;

    // Allocate new descriptors
    desc_hdr = virtio_desc_alloc(&desc_hdr_index);
    desc_data = virtio_desc_alloc(&desc_data_index);
    desc_status = virtio_desc_alloc(&desc_status_index);

    // Header request
    desc_hdr->addr = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)req);
    desc_hdr->length = sizeof(virtio_blk_req_t);
    desc_hdr->flags = VIRTIO_DESC_F_NEXT;
    desc_hdr->next = desc_data_index;

    // Buffer data
    desc_data->addr = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)buf);
    desc_data->length = length;
    desc_data->flags = VIRTIO_DESC_F_WRITE | VIRTIO_DESC_F_NEXT;
    desc_data->next = desc_status_index;

    // Status
    desc_status->addr = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)status);
    desc_status->length = 1;
    desc_status->flags = VIRTIO_DESC_F_WRITE;
    desc_status->next = 0;

    virtio_submit_req(desc_hdr_index);

    while(*status == 0xFF) {
        // Wait for disk to complete the request
        if (*status != 0xFF) {
            break;
        }
    }

    uint32_t st = *status;

    if (st == VIRTIO_BLK_S_IOERR) {
        printf("VIRTIO: Failed to read sector 0x%lx! (IO error)\n", sector);

        kfree(req, sizeof(virtio_blk_req_t));
        kfree(status, 1);
        spinlock_release(&s);
        return st;
    }

    if (st == VIRTIO_BLK_S_UNSUPP) {
        printf("VIRTIO: Failed to read sector 0x%lx! (unsupported)\n", sector);

        kfree(req, sizeof(virtio_blk_req_t));
        kfree(status, 1);
        spinlock_release(&s);
        return st;
    }

    kfree(req, sizeof(virtio_blk_req_t));
    kfree(status, 1);
    spinlock_release(&s);
    return 0;
}

uint32_t virtio_blk_write(uint8_t *buf, uint64_t sector, uint64_t length) {
    uint8_t *status = NULL;
    virtio_blk_req_t *req = NULL;
    virtio_desc_t *desc_hdr = NULL, *desc_data = NULL, *desc_status = NULL;
    uint32_t desc_hdr_index = 0, desc_data_index = 0, desc_status_index = 0;



    assert(buf != NULL);

    spinlock_acquire(&s);

    // Pad to 512 bytes
    length = virtio_pad_bytes(length);

    req = kmalloc(sizeof(virtio_blk_req_t));
    assert(req != NULL);

    // Write command
    req->type = VIRTIO_BLK_T_OUT;
    req->sector = sector;
    req->reserved = 0;

    status = kmalloc(1);
    assert(status != NULL);

    *status = 0;

    // Allocate new descriptors
    desc_hdr = virtio_desc_alloc(&desc_hdr_index);
    desc_data = virtio_desc_alloc(&desc_data_index);
    desc_status = virtio_desc_alloc(&desc_status_index);

    // Header request
    desc_hdr->addr = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)req);
    desc_hdr->length = sizeof(virtio_blk_req_t);
    desc_hdr->flags = VIRTIO_DESC_F_NEXT;
    desc_hdr->next = desc_data_index;

    // Buffer data
    desc_data->addr = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)buf);
    desc_data->length = length;
    desc_data->flags = VIRTIO_DESC_F_NEXT;
    desc_data->next = desc_status_index;

    // Status
    desc_status->addr = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)status);
    desc_status->length = 1;
    desc_status->flags = VIRTIO_DESC_F_WRITE;
    desc_status->next = 0;


    virtio_submit_req(desc_hdr_index);

    while(*status == 0xFF) {
        // Wait for disk to complete the request
        if (*status != 0xFF) {
            break;
        }
    }

    uint32_t st = *status;

    if (st == VIRTIO_BLK_S_IOERR) {
        printf("VIRTIO: Failed to write sector 0x%lx! (IO error)\n", sector);

        kfree(req, sizeof(virtio_blk_req_t));
        kfree(status, 1);
        spinlock_release(&s);
        return st;
    }

    if (st == VIRTIO_BLK_S_UNSUPP) {
        printf("VIRTIO: Failed to write sector 0x%lx! (unsupported)\n", sector);

        kfree(req, sizeof(virtio_blk_req_t));
        kfree(status, 1);
        spinlock_release(&s);
        return st;
    }

    kfree(req, sizeof(virtio_blk_req_t));
    kfree(status, 1);
    spinlock_release(&s);
    return 0;
}

virtio_queue_t *virtio_blk_queue_init(size_t queue_size) {
    virtio_queue_t *vq = NULL;
    size_t desc_size = VIRTIO_DESC_ALIGN(sizeof(virtio_desc_t) * queue_size);
    size_t avail_size = VIRTIO_AVAIL_ALIGN(sizeof(virtio_avail_t) + sizeof(uint16_t) * queue_size);
    size_t used_size = VIRTIO_USED_ALIGN(sizeof(virtio_used_t) + sizeof(virtio_used_elem_t) * desc_size);
    size_t total_size = VIRTIO_ALIGN(desc_size + avail_size + used_size, PAGE_SIZE);


    
    // TODO: Cleanup MDL for kamlloc(), etc...
    vq_mdl = vmm_mdl_alloc(total_size);
    assert(vq_mdl != NULL);

    vq = vmm_mdl_get_page(vq_mdl, 0);

    for (size_t i = 0; i < queue_size; i++) {
        vq->desc[i].next = 1 + i;
    }

    vq->free_desc = 0;
    vq->last_used = 0;

    return vq;
}

virtio_desc_t *virtio_desc_alloc(uint32_t *idx) {
    virtio_desc_t *desc = NULL;
    uint16_t index = -1, next_index = -1;



    assert(vq != NULL);

    index = vq->free_desc;

    if (index == VIRTIO_QUEUE_SIZE) {
        printf("VIRTIO: No available queue descriptors!\n");
        return NULL;
    }

    desc = (virtio_desc_t *)&vq->desc[index];
    desc->addr = 0;
    desc->length = 0;
    desc->flags = 0;

    // Get next descriptor index
    next_index = desc->next;

    // Update next available descriptor
    vq->free_desc = next_index;

    *idx = index;
    return desc;
}

void virtio_desc_free(uint16_t index) {
    virtio_desc_t *desc = NULL;


    assert(vq != NULL);

    desc = (virtio_desc_t *)&vq->desc[index];
    
    desc->next = vq->free_desc;
    vq->free_desc = index;
}

void virtio_desc_free_multiple(uint16_t desc_index) {
    assert(vq != NULL);

    while(vq->desc[desc_index].flags & VIRTIO_DESC_F_NEXT) {
        uint16_t next = vq->desc[desc_index].next;

        virtio_desc_free(next);
        desc_index = next;
    }

    virtio_desc_free(desc_index);
}