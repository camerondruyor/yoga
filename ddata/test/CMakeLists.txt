cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)

set(SOURCES
        AllocatorTests.cpp
        ComplexDifferentiationTests.cpp
        ETDOperatorTests.cpp
        ETDFunctionTests.cpp
        SmoothFunctionsTests.cpp
        StackBasedTests.cpp
        StackOperatorTests.cpp
        StackFunctionTests.cpp
        StackJacobianTests.cpp
        TestHelpers.h
        )

if (Kokkos-simd_FOUND)
    set(SOURCES ${SOURCES} VTDTests.cpp)
endif()
add_catch_unit_test(ddata_UnitTests ${SOURCES})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(ddata_UnitTests PRIVATE ${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-int-in-bool-context -Wno-misleading-indentation)
endif()
if (CMAKE_CXX_COMPILER MATCHES nvcc_wrapper)
    # To avoid annoying, but harmless, warning messages
    target_compile_options(ddata_UnitTests PRIVATE --host-only)
endif()

target_link_libraries(ddata_UnitTests PRIVATE ddata::ddata)
