target_sources(${KERNEL_TARGET} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/putc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/puts.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/printf.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/snprintf.c"
)