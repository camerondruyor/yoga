cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(sources
        CellCenteredReProcessor.cpp
        )

set(headers
        CellCenteredReProcessor.h
)

add_library(CC_ReProcessor SHARED ${sources} ${headers})
add_library(parfait::CC_ReProcessor ALIAS CC_ReProcessor)
target_include_directories(CC_ReProcessor
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:include>
        )
if(ParMETIS_FOUND)
    target_link_libraries(CC_ReProcessor PUBLIC ParMETIS::ParMETIS )
    target_compile_definitions(CC_ReProcessor PRIVATE HAVE_PARMETIS)
endif()

target_link_libraries(CC_ReProcessor
        PUBLIC
        infinity::infinity
        parfait::NC_PreProcessor
        PRIVATE
        MessagePasser::MessagePasser
        tracer::tracer
        parfait::parfait
        parfait::S_PreProcessor
        )

set_standard_ring_rpath(CC_ReProcessor)

install(TARGETS CC_ReProcessor
        EXPORT CC_ReProcessor
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

install(EXPORT CC_ReProcessor NAMESPACE parfait:: DESTINATION cmake)

install(FILES ${headers} DESTINATION include/CellCenteredReProcessor)

