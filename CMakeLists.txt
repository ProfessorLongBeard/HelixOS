cmake_minimum_required(VERSION 3.22.1)

project(HelixOS LANGUAGES C ASM)

if(WIN32)
    message(FATAL_ERROR "HelixOS does not support being built in Windows environments!")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(HELIX_ARCH "arm64" CACHE STRING "")

if(NOT HELIX_ARCH)
    message(FATAL_ERROR "HELIX_ARCH not defined!")

elseif(NOT HELIX_ARCH STREQUAL "arm64")
    message(FATAL_ERROR "Unsupported architecture: ${HELIX_ARCH}")

endif()

include(toolchain-${HELIX_ARCH})



if (NOT EXISTS $ENV{HELIX_SYSROOT})
    file(MAKE_DIRECTORY $ENV{HELIX_SYSROOT})

    if (NOT EXISTS $ENV{HELIX_PREFIX})
        file(MAKE_DIRECTORY $ENV{HELIX_PREFIX})
    endif()
endif()

add_subdirectory(kernel)
add_subdirectory(scripts)