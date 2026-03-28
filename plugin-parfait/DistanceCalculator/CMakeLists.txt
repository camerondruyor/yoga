cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(headers DistanceCalculatorPlugin.h)

set(sources DistanceCalculatorPlugin.cpp)

add_library(ParfaitDistanceCalculator SHARED ${sources} ${headers})
add_library(parfait::ParfaitDistanceCalculator ALIAS ParfaitDistanceCalculator)
target_include_directories(ParfaitDistanceCalculator PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        )

target_link_libraries(ParfaitDistanceCalculator PUBLIC
        tracer::tracer
        parfait::parfait
        infinity::infinity
        )

set_standard_ring_rpath(ParfaitDistanceCalculator)

install(TARGETS ParfaitDistanceCalculator
        EXPORT ParfaitDistanceCalculator
        LIBRARY DESTINATION lib)

install(EXPORT ParfaitDistanceCalculator
        NAMESPACE parfait::
        DESTINATION cmake)
