#include <kstdio.h>
#include <kstdlib.h>
#include <irq.h>
#include <mm.h>
#include <arch.h>
#include <devices/gicv3.h>
#include <devices/virtio/virtio.h>





static volatile virtio_t *v = (virtio_t *)VIRTIO_MMIO_BASE;

static virtio_queue_t *vq = NULL;


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

    vq = virtio_queue_init(VIRTIO_QUEUE_SIZE);
    assert(vq != NULL);

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

    irq_register(VIRTIO_IRQ_ID, virtio_irq_handler);
    gic_set_irq_group_ns(VIRTIO_IRQ_ID);
    gic_set_irq_level_trigger(VIRTIO_IRQ_ID);
    gic_enable_irq(VIRTIO_IRQ_ID);

    printf("VIRTIO: Device intialized!\n");
}

void virtio_irq_handler(void) {
    printf("VIRTIO: IRQ Handler called!\n");
}

void virtio_test(void) {
    virtio_blk_req_t *req = kmalloc(sizeof(virtio_blk_req_t));
    assert(req != NULL);

    uint8_t *data = kmalloc(512);
    assert(data != NULL);

    memset(data, 0, 512);

    uint8_t *st = kmalloc(1);
    assert(st != NULL);
    *st = 0;

    uint32_t irq_status_before = v->interrupt_status;

    printf("IRQ status before: 0x%lx\n", irq_status_before);

    req->type = VIRTIO_BLK_T_IN;
    req->reserved = 0;
    req->sector = 0;

    vq->desc[0].addr = VIRT_TO_PHYS((uintptr_t)req);
    vq->desc[0].length = sizeof(virtio_blk_req_t);
    vq->desc[0].flags = VIRTIO_DESC_F_NEXT;
    vq->desc[0].next = 1;

    vq->desc[1].addr = VIRT_TO_PHYS((uintptr_t)data);
    vq->desc[1].length = 512;
    vq->desc[1].flags = VIRTIO_DESC_F_WRITE | VIRTIO_DESC_F_NEXT;
    vq->desc[1].next = 2;

    vq->desc[2].addr = VIRT_TO_PHYS((uintptr_t)st);
    vq->desc[2].length = 1;
    vq->desc[2].flags = VIRTIO_DESC_F_WRITE;
    vq->desc[2].next = 0;

    v->queue_sel = 0;
    __mb();

    v->queue_num = VIRTIO_QUEUE_SIZE;

    uintptr_t desc_phys = VIRT_TO_PHYS((uintptr_t)vq->desc);
    uintptr_t avail_phys = VIRT_TO_PHYS((uintptr_t)&vq->avail);
    uintptr_t used_phys = VIRT_TO_PHYS((uintptr_t)&vq->used);

    __mb();
    v->queue_desc_low = desc_phys;
    v->queue_desc_high = 0;

    v->queue_avail_low = avail_phys;
    v->queue_avail_high = 0;

    v->queue_used_low = used_phys;
    v->queue_used_high = 0;
    __mb();

    v->queue_ready = 1;
    __mb();

    vq->avail.ring[vq->avail.index % VIRTIO_QUEUE_SIZE] = 0;

    vq->avail.flags = RING_EVENT_FLAGS_ENABLE;
    vq->avail.event = 0;

    vq->used.flags = RING_EVENT_FLAGS_ENABLE;
    vq->used.event = 0;

    __mb();
    vq->avail.index += 1;
    __mb();

    v->queue_notify = 0;

    uint64_t timeout = 100000;
    while(timeout-- > 0);

    uint32_t irq_status = v->interrupt_status;

    printf("IRQ status after: 0x%lx\n", irq_status);

    printf("Data: 0x%lx 0x%lx\n", data[510], data[511]);
}