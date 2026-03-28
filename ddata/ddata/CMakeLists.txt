cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(HEADERS
        ComplexDifferentiation.h
        CustomAllocator.h
        ETDExpression.h
        DTD.h
        ETD.h
        ETDv.h
        ETDOperators.h
        ETDFunctions.h
        ETDStack.h
        Expr.h
        StackFunctions.h
        StackOperators.h
        Stack.h
        SmoothFunctions.h
        )

add_library(ddata INTERFACE)
add_library(ddata::ddata ALIAS ddata)
target_include_directories(ddata INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
        )

if (Kokkos-simd_FOUND)
    message("Found Kokkos SIMD")
    set(HEADERS ${HEADERS} VTD.h)
    target_link_libraries(ddata INTERFACE Kokkos::simd)
endif()

install(TARGETS ddata
        EXPORT ddata
        LIBRARY DESTINATION lib)

install(FILES ${HEADERS}
        DESTINATION include/ddata)

install(EXPORT ddata
NAMESPACE ddata:: DESTINATION cmake)

install(FILES ddataConfig.cmake
        DESTINATION cmake)
