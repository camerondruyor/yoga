cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(yoga)
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(one-ring-common)
include(CompilerOptions)

get_compiler_warning_options(compile_warning_options)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${compile_warning_options}")

get_filename_component(path_to_yoga "." ABSOLUTE)

add_definitions(
    -DCATCH_MPI_ENABLED
    -DYOGA_WITH_TRACER
    -DOMPI_SKIP_MPICXX
    -DMPI_NO_CPPBIND
     )
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
include(FetchContent)
FetchContent_Declare(
    nanoflann
    GIT_REPOSITORY https://github.com/jlblancoc/nanoflann.git
    GIT_TAG        v1.6.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nanoflann)
find_package(tracer REQUIRED)
find_package(MessagePasser REQUIRED)
find_package(parfait REQUIRED)
#find_library(ZeroMQ_LIBRARY QUIET
#    NAMES zmq libzmq
#    )
#find_path(ZeroMQ_INCLUDE_DIR QUIET
#  NAMES zmq.h
#  )

find_package(t-infinity REQUIRED)
find_package(ddata REQUIRED)
find_package(ZeroMQ QUIET)
if(ZeroMQ_FOUND)
    message("Found zmq, enabling zmq path")
    add_definitions(
        -DYOGA_WITH_ZMQ
    )
    set(YOGA_LINK_ZMQ TRUE)
else()
    message("Could not find zmq, only building mpi version")
endif()

if(Catch2_FIND_QUIETLY)
    set(BUILD_YOGA_TESTS TRUE CACHE BOOL "builds unit tests, requires Catch2")
endif()

add_subdirectory(src)


if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND RING_BUILD_TESTING)
    include(CTest)
    enable_testing()
endif()

if(RING_BUILD_TESTING)
    add_subdirectory(tests)
    if (NOT RING_ENABLE_CUDA)
        # Nothing CUDA-related needs testing here
        add_subdirectory(regression/mixed_element_sphere_box)
    endif()
endif()
add_subdirectory(utilities)
