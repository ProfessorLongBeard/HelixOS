#include <kstdio.h>
#include <kstdlib.h>
#include <mm.h>
#include <devices/virtio/virtio.h>









size_t virtio_get_virtq_size(size_t queue_length) {
    return VIRTIO_ALIGN(sizeof(virtio_desc_t) * queue_length + sizeof(uint16_t) * (3 + queue_length), PAGE_SIZE)
            + VIRTIO_ALIGN(sizeof(uint16_t) * 3 + sizeof(virtio_used_elem_t) * queue_length, PAGE_SIZE);
}

virtio_queue_t *virtio_queue_init(size_t queue_size) {
    virtio_queue_t *vq = NULL;
    size_t desc_size = VIRTIO_DESC_ALIGN(sizeof(virtio_desc_t) * queue_size);
    size_t avail_size = VIRTIO_AVAIL_ALIGN(sizeof(virtio_avail_t) + sizeof(uint16_t) * queue_size);
    size_t used_size = VIRTIO_USED_ALIGN(sizeof(virtio_used_t) + sizeof(virtio_used_elem_t) * desc_size);
    uintptr_t desc_offset = desc_size;
    uintptr_t avail_offset = desc_offset + avail_size;
    uintptr_t used_offset = avail_offset + used_size;
    size_t total_size = VIRTIO_ALIGN(desc_size + avail_size + used_size, PAGE_SIZE);
    
    // // TODO: Use kmalloc() when allocators are more stable!
    vq = pmm_alloc_pages(SIZE_TO_PAGES(total_size, PAGE_SIZE));
    assert(vq != NULL);

    for (size_t i = 0; i < queue_size; i++) {
        vq->desc[i].next = 1 + i;
    }

    return vq;
}