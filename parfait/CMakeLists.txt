cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(one-ring-common)
include(CompilerOptions)

PROJECT(parfait)
set (CMAKE_CXX_STANDARD 14)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(Catch2_path "Specify search path to catch.hpp" "")

get_compiler_warning_options(compile_warning_options)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${compile_warning_options}")

find_package(MessagePasser QUIET)

add_subdirectory(parfait)

if (MessagePasser_FOUND)
  if (RING_BUILD_TESTING)
    add_subdirectory(parfait/test)
  endif ()
endif()

install(FILES parfaitConfig.cmake
        DESTINATION cmake)
