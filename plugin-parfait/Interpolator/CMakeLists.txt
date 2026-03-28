cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(headers InterpolatePlugin.h)

set(sources InterpolatePlugin.cpp)

add_library(ParfaitInterpolator SHARED ${sources} ${headers})
add_library(parfait::ParfaitInterpolator ALIAS ParfaitInterpolator)
target_include_directories(ParfaitInterpolator PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        )

target_link_libraries(ParfaitInterpolator PUBLIC
        tracer::tracer
        parfait::parfait
        infinity::infinity
        )

set_standard_ring_rpath(ParfaitInterpolator)

install(TARGETS ParfaitInterpolator
    EXPORT ParfaitInterpolator
    LIBRARY DESTINATION lib)

install(EXPORT ParfaitInterpolator
        NAMESPACE parfait::
        DESTINATION cmake)
