set(KERNEL_TARGET "kernel.elf" CACHE STRING "")



add_executable(${KERNEL_TARGET})

target_include_directories(${KERNEL_TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/external/limine

)

target_compile_options(${KERNEL_TARGET} PRIVATE
    -g
    -O0
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-unused-command-line-argument
    -fno-stack-protector
    -fno-stack-check
    -ffreestanding
    -fno-PIC
)

target_link_options(${KERNEL_TARGET} PRIVATE
    -Wl,--build-id=none
    -nostdlib
    -static
    -T ${CMAKE_SOURCE_DIR}/kernel/arch/${HELIX_ARCH}/helix.ld
)

add_subdirectory(arch)
add_subdirectory(init)