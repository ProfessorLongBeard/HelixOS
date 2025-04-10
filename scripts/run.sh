#!/bin/sh

# name "virtio-rng-device", bus virtio-bus
# name "virtio-rng-pci", bus PCI, alias "virtio-rng"

# name "virtio-sound-device", bus virtio-bus
# name "virtio-sound-pci", bus PCI, alias "virtio-sound", desc "Virtio Sound"

# name "virtio-gpu-device", bus virtio-bus
# name "virtio-gpu-pci", bus PCI, alias "virtio-gpu"

# name "virtio-input-host-device", bus virtio-bus
# name "virtio-input-host-pci", bus PCI, alias "virtio-input-host"
# name "virtio-keyboard-device", bus virtio-bus
# name "virtio-keyboard-pci", bus PCI, alias "virtio-keyboard"
# name "virtio-mouse-device", bus virtio-bus
# name "virtio-mouse-pci", bus PCI, alias "virtio-mouse"

# name "virtio-net-device", bus virtio-bus
# name "virtio-net-pci", bus PCI, alias "virtio-net"
# name "virtio-net-pci-non-transitional", bus PCI
# name "virtio-net-pci-transitional", bus PCI

# name "virtio-blk-device", bus virtio-bus
# name "virtio-blk-pci", bus PCI, alias "virtio-blk"
# name "virtio-blk-pci-non-transitional", bus PCI
# name "virtio-blk-pci-transitional", bus PCI

# name "sd-card", bus sd-bus
# name "sd-card-spi", bus sd-bus, desc "SD SPI"
# name "sdhci-pci", bus PCI






OUT_DIR=${HELIX_BINARY_DIR}

EFI_ROM=${HELIX_BINARY_DIR}/efi_rom.img
EFI_VARS=${HELIX_BINARY_DIR}/efi_vars.img
HDD_IMG=${HELIX_BINARY_DIR}/helix_hdd.img

QEMU=qemu-system-aarch64

MACHINE=virt,gic-version=3
CPU=cortex-a53
SMP=1
MEM=4G



${QEMU} \
    -d guest_errors,cpu_reset,mmu \
    -M ${MACHINE} \
    -cpu ${CPU} \
    -smp ${SMP} \
    -m ${MEM} \
    -drive if=pflash,file=${EFI_ROM},format=raw,readonly=on \
    -drive if=pflash,file=${EFI_VARS},format=raw,readonly=off \
    -drive if=none,file=${HDD_IMG},format=raw,id=vda1 \
    -device virtio-blk-device,drive=vda1,bus=virtio-mmio-bus.0 \
    -global virtio-mmio.force-legacy=false \
    -usb \
    -device qemu-xhci \
    -device usb-kbd \
    -device usb-mouse \
    -net none \
    -device ramfb \
    -display gtk,zoom-to-fit=off
