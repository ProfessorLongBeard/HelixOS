target_sources(${KERNEL_TARGET} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/memset.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/memcpy.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/strlen.c"
)