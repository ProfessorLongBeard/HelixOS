#!/bin/sh


OUT_DIR=${HELIX_BINARY_DIR}

EFI_ROM=${HELIX_BINARY_DIR}/efi_rom.img
EFI_VARS=${HELIX_BINARY_DIR}/efi_vars.img
HDD_IMG=${HELIX_BINARY_DIR}/helix_hdd.img

QEMU=qemu-system-aarch64

MACHINE=virt,gic-version=3
CPU=cortex-a53
SMP=1
MEM=4G



ln -sf ${HELIX_SOURCE_DIR}/.gdbinit ${HELIX_BINARY_DIR}/scripts/



${QEMU} \
    -s -S \
    -d guest_errors,cpu_reset,int \
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