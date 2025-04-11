target_sources(${KERNEL_TARGET} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/gicv3.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/pl011.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/timer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc.c"
)

add_subdirectory(virtio)