target_sources(${KERNEL_TARGET} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/start.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/vectors.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/asm.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/arch.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/exc_handler.c"
)