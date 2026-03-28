cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(infinity)
set (CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(one-ring-common)
include(CompilerOptions)

get_compiler_warning_options(compile_warning_options)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${compile_warning_options}")

if(ENABLE_CUDA)
  set(BUILD_PYTHON_RUNTIME false)
endif()

find_package(MessagePasser REQUIRED)
find_package(parfait REQUIRED)
find_package(tracer REQUIRED)

add_subdirectory(src)

if (RING_BUILD_TESTING AND BUILD_RUNTIME)
    add_subdirectory(examples)
endif()

# FIXME: Disabled for asan build, due to "symbol not found" error
if (RING_BUILD_TESTING AND BUILD_PYTHON_RUNTIME AND NOT CMAKE_BUILD_TYPE MATCHES asan AND NOT RING_ENABLE_CUDA)
    add_test(NAME t-infinity-python-unit-tests COMMAND ./run_python_unit_tests.sh ${CMAKE_INSTALL_PREFIX}
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    set_tests_properties(t-infinity-python-unit-tests PROPERTIES PROCESSORS 1)
endif()
