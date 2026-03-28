cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(ddata)

set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(one-ring-common)

find_package(Kokkos-simd QUIET)

add_subdirectory(ddata)

if (RING_BUILD_TESTING)
    add_subdirectory(test)
    if (NOT RING_ENABLE_CUDA)
      add_subdirectory(miniapp)
    endif()
endif()
