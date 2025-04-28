#include <kstdio.h>
#include <kstdlib.h>
#include <irq.h>
#include <mm/mm.h>
#include <arch.h>
#include <devices/gicv3.h>
#include <devices/virtio/virtio.h>
#include <devices/virtio/virtio_blk.h>





static volatile virtio_t *v = (virtio_t *)VIRTIO_MMIO_BASE;




/*
 * Device initialization:
 *
 * - Reset the device
 * - Set the acknowledge bit
 * - Set the driver status bit
 * - Read the device features bits, and write a subset of them understood by the OS
 * - Set the features_ok status bit, read-read to ensure the bit is till set (otherwise feature(s) are not supported)
 * - Perform device-specific setup, includes virtqueues discovery, writing devices virtio configuration space, etc
 * - Set the driver_ok status bit (device should now be "live")
 */





void virtio_init(void) {
    if (v->magic != VIRTIO_MAGIC) {
        printf("VIRTIO: Invalid device magic: 0x%lx (expected: 0x%lx)\n", v->magic, VIRTIO_MAGIC);
        return;
    }

    if (v->version != VIRTIO_VERSION) {
        if (v->version == VIRTIO_VERSION_LEGACY) {
            printf("VIRTIO: Legacy virtio not supported!\n");
            return;
        }

        printf("VIRTIO: Invalid virtio version: 0x%lx (expected: 0x%lx)\n", v->version, VIRTIO_VERSION);
        return;
    }

    // Reset the device
    v->status = VIRTIO_DEVICE_RESET;
    __mb();

    // Wait for device to be reset
    while(v->status & VIRTIO_DEVICE_RESET);

    // Acknowledge the reset
    v->status = VIRTIO_DEVICE_ACK;
    __mb();

    if (!(v->status & VIRTIO_DEVICE_ACK)) {
        if (v->status & VIRTIO_DEVICE_FAILED) {
            printf("VIRTIO: Failed to acknowledge device reset!\n");
            return;
        }

        if (v->status & VIRTIO_DEVICE_NEEDS_RESET) {
            printf("VIRTIO: Failed to acknowledge device reset (needs reset)!\n");
            return;
        }

        printf("VIRTIO: Unknown error occured during reset acknowledgement!\n");
        return;
    }

    // Put device in driver mode
    v->status |= VIRTIO_DEVICE_DRIVER;
    __mb();

    if (!(v->status & VIRTIO_DEVICE_DRIVER)) {
        if (v->status & VIRTIO_DEVICE_FAILED) {
            printf("VIRTIO: Failed to set device driver mode!\n");
            return;
        }

        if (v->status & VIRTIO_DEVICE_NEEDS_RESET) {
            printf("VIRTIO: Failed to set device driver mode (needs reset)!\n");
            return;
        }

        printf("Unknown error occured setting device driver mode!\n");
        return;
    }

    // Setup device features
    uint32_t device_feature_sel = (uint32_t)(1ULL << VIRTIO_BLK_F_VERSION_1);

    device_feature_sel |= (uint32_t)(1UL << VIRTIO_BLK_F_FLUSH);
    device_feature_sel |= (uint32_t)(1UL << VIRTIO_BLK_F_RO);

    v->driver_features = device_feature_sel;
    __mb();
    
    v->driver_features_sel = 0;
    v->device_features_sel = 0;

    v->status |= VIRTIO_DEVICE_FEATURES_OK;
    __mb();

    if (!(v->status & VIRTIO_DEVICE_FEATURES_OK)) {
        if (v->status & VIRTIO_DEVICE_FAILED) {
            printf("VIRTIO: Failed to set device/driver feature(s)!\n");
            return;
        }

        if (v->status & VIRTIO_DEVICE_NEEDS_RESET) {
            printf("VIRTIO: Failed to set device/driver feature(s) (needs reset)!\n");
            return;
        }

        printf("VIRTIO: Unknown error occured setting device/driver features!\n");
        return;
    }

    v->status |= VIRTIO_DEVICE_DRIVER_OK;
    __mb();

    if (!(v->status & VIRTIO_DEVICE_DRIVER_OK)) {
        if (v->status & VIRTIO_DEVICE_FAILED) {
            printf("VIRTIO: Failed to set the device as live!\n");
            return;
        }

        if (v->status & VIRTIO_DEVICE_NEEDS_RESET) {
            printf("VIRTIO: Failed to set the device as live (needs reset)!\n");
            return;
        }

        printf("VIRTIO: Unknown error has occured setting the device as live!\n");
        return;
    }

    v->queue_num = VIRTIO_QUEUE_SIZE;
    __mb();

    virtio_blk_init();

    irq_register(VIRTIO_IRQ_ID, virtio_irq_handler);
    gic_set_irq_group_ns(VIRTIO_IRQ_ID);
    gic_set_irq_level_trigger(VIRTIO_IRQ_ID);
    gic_enable_irq(VIRTIO_IRQ_ID);

    printf("VIRTIO: Device intialized!\n");
}

void virtio_irq_handler(void) {
    virtio_used_elem_t *elem = NULL;
    virtio_blk_req_t *req = NULL;
    uint16_t last_used = 0, used_index = 0, ring_index = 0;
    uint16_t desc1 = 0, desc2 = 0, desc3 = 0;




    v->interrupt_ack = v->interrupt_status;

    last_used = vq->last_used;
    used_index = vq->used.index;

    while(last_used != used_index) {
        ring_index = last_used % VIRTIO_QUEUE_SIZE;
        elem = (virtio_used_elem_t *)&vq->used.ring[ring_index];

        desc1 = elem->id;

        if (!(vq->desc[desc1].flags & VIRTIO_DESC_F_NEXT)) {
            printf("VIRTIO: Header descriptor malformed!\n");
            
            last_used++;
            continue;
        }

        desc2 = vq->desc[desc1].next;

        if (!(vq->desc[desc2].flags & VIRTIO_DESC_F_NEXT)) {
            printf("VIRTIO: Data descrioptor malformed!\n");
            
            last_used++;
            continue;
        }

        desc3 = vq->desc[desc2].next;

        if (vq->desc[desc3].length != 1) {
            printf("VIRTIO: Status descriptor length mismatch!\n");
            
            last_used++;
            continue;
        }

        virtio_desc_free(desc1);
        virtio_desc_free(desc2);
        virtio_desc_free(desc3);

        last_used++;
    }

    vq->last_used = last_used;
}

void virtio_submit_req(uint16_t desc_index) {
    uint16_t ring_index = 0;
    uintptr_t desc_phys = 0, avail_phys = 0, used_phys = 0;




    v->queue_sel = desc_index;

    v->queue_num = VIRTIO_QUEUE_SIZE;

    desc_phys = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)vq->desc);
    avail_phys = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)&vq->avail);
    used_phys = vmm_virt2phys(vmm_get_pgd(), (uintptr_t)&vq->used);

    v->queue_desc_low = desc_phys;
    v->queue_desc_high = 0;

    v->queue_avail_low = avail_phys;
    v->queue_avail_high = 0;

    v->queue_used_low = used_phys;
    v->queue_used_high = 0;
    __mb();

    vq->avail.ring[vq->avail.index % VIRTIO_QUEUE_SIZE] = desc_index;

    vq->avail.flags = RING_EVENT_FLAGS_ENABLE;
    vq->avail.event = 0;

    vq->used.flags = RING_EVENT_FLAGS_ENABLE;
    vq->used.event = 0;
    __mb();

    v->queue_ready = 1;

    __mb();
    vq->avail.index++;
    __mb();

    v->queue_notify = 0;
    __mb();
}