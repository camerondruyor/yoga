cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(HEADER_FILES
        SerialPreProcessor.h
        )

set(SRC
        SerialPreProcessor.cpp
        )


add_library(S_PreProcessor SHARED ${SRC} ${HEADER_FILES})
add_library(parfait::S_PreProcessor ALIAS S_PreProcessor)
target_include_directories(S_PreProcessor PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        )

target_link_libraries(S_PreProcessor PUBLIC
        tracer::tracer
        parfait::parfait
        infinity::infinity
        plugin_parfait_core
        )

set_standard_ring_rpath(S_PreProcessor)

install(FILES ${HEADER_FILES} DESTINATION include/SerialPreProcessor.h)

install(TARGETS S_PreProcessor
    EXPORT S_PreProcessor
    LIBRARY DESTINATION lib)

install(EXPORT S_PreProcessor
        NAMESPACE parfait::
        DESTINATION cmake)

