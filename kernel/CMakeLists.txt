set(KERNEL_TARGET "kernel.elf" CACHE STRING "")



add_executable(${KERNEL_TARGET})

target_include_directories(${KERNEL_TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/external/limine
    ${CMAKE_SOURCE_DIR}/externel/flanterm
    ${CMAKE_SOURCE_DIR}/external/freestnd-c-hdrs/aarch64/include

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

if (EXISTS $ENV{HELIX_EXT_DIR}/flanterm)
    if (NOT EXISTS $ENV{HELIX_EXT_DIR}/flanterm/CMakeLists.txt)
        configure_file(${CMAKE_SOURCE_DIR}/cmake/flanterm.cmake.in $ENV{HELIX_EXT_DIR}/flanterm/CMakeLists.txt @ONLY)
    endif()
    
    add_subdirectory($ENV{HELIX_EXT_DIR}/flanterm ${CMAKE_BINARY_DIR}/flanterm)
endif()

add_subdirectory(arch)
add_subdirectory(init)
add_subdirectory(devices)