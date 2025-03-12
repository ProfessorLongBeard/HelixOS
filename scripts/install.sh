#!/bin/sh



DISK_IMG=${HELIX_BINARY_DIR}/helix_hdd.img
EFI_IMG=${HELIX_BINARY_DIR}/efi_rom.img
EFI_VARS=${HELIX_BINARY_DIR}/efi_vars.img

HDD_BLOCK_SIZE=512

EFI_PARTITION_START=2048
EFI_PARTITION_OFFSET=$((${HDD_BLOCK_SIZE} * ${EFI_PARTITION_START}))






[ ! -e ${DISK_IMG} ] && qemu-img create -f raw ${DISK_IMG} 4G

[ ! -e ${EFI_IMG} ] && qemu-img create -f raw ${EFI_IMG} 64M && dd if=${HELIX_EXT_DIR}/qemu_arm64_efi.fd of=${EFI_IMG} conv=notrunc
[ ! -e ${EFI_VARS} ] && qemu-img create -f raw ${EFI_VARS} 64M



sgdisk \
    --new 1:0:+512M \
    --typecode 1:"ef00" \
    --change-name 1:"EFI" \
    ${DISK_IMG} >/dev/null 2>&1


mformat \
    -T 1048576 \
    -h 32 \
    -s 63 \
    -H 0 \
    -i ${DISK_IMG}@@${EFI_PARTITION_OFFSET} ::/


mmd -i ${DISK_IMG}@@${EFI_PARTITION_OFFSET} ::/EFI
mmd -i ${DISK_IMG}@@${EFI_PARTITION_OFFSET} ::/EFI/BOOT

mmd -i ${DISK_IMG}@@${EFI_PARTITION_OFFSET} ::/HelixOS
mmd -i ${DISK_IMG}@@${EFI_PARTITION_OFFSET} ::/HelixOS/images
mmd -i ${DISK_IMG}@@${EFI_PARTITION_OFFSET} ::/HelixOS/fonts

mcopy -i ${DISK_IMG}@@${EFI_PARTITION_OFFSET} ${HELIX_EXT_DIR}/limine/BOOTAA64.EFI ::/EFI/BOOT/BOOTAA64.efi
mcopy -i ${DISK_IMG}@@${EFI_PARTITION_OFFSET} ${HELIX_SOURCE_DIR}/limine.conf ::/EFI/BOOT/

mcopy -i ${DISK_IMG}@@${EFI_PARTITION_OFFSET} ${HELIX_BINARY_DIR}/kernel/kernel.elf ::/HelixOS/
mcopy -i ${DISK_IMG}@@${EFI_PARTITION_OFFSET} ${HELIX_EXT_DIR}/images/default.bmp ::/HelixOS/images/boot_splash.bmp