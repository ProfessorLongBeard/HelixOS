#!/bin/sh


OUT_DIR=${HELIX_BINARY_DIR}

EFI_ROM=${HELIX_BINARY_DIR}/efi_rom.img
EFI_VARS=${HELIX_BINARY_DIR}/efi_vars.img
HDD_IMG=${HELIX_BINARY_DIR}/helix_hdd.img

QEMU=qemu-system-aarch64

MACHINE=virt
CPU=cortex-a53
SMP=1
MEM=4G



ln -sf ${HELIX_SOURCE_DIR}/.gdbinit ${HELIX_BINARY_DIR}/scripts/



${QEMU} \
    -s -S \
    -d guest_errors,cpu_reset \
    -M ${MACHINE} \
    -cpu ${CPU} \
    -smp ${SMP} \
    -m ${MEM} \
    -drive if=pflash,file=${EFI_ROM},format=raw,readonly=on \
    -drive if=pflash,file=${EFI_VARS},format=raw,readonly=off \
    -drive if=virtio,file=${HDD_IMG},format=raw \
    -usb \
    -device qemu-xhci \
    -device usb-kbd \
    -device usb-mouse \
    -net none \
    -device ramfb \
    -serial stdio \
    -display gtk,zoom-to-fit=off