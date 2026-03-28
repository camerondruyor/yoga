cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set (CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(one-ring-common)

set(TEST_SOURCES
        CGNSReaderTests.cpp
        ElementCounterTests.cpp
        MockReaders.h
        OneTetMesh.h
        SliceRecipeTests.cpp
        )


add_catch_unit_test(CGNSReaderTests ${TEST_SOURCES} ../../CGNSReader.cpp ../../ElementCounter.cpp)
target_link_libraries(CGNSReaderTests PRIVATE
        CGNS::CGNS
        parfait::parfait
        infinity::infinity)

target_include_directories(CGNSReaderTests PUBLIC ${CGNS_INCLUDE_DIRS} ../../)

# Disabled because I can't figure out how to find the location of the grid files in ctest
#enable_testing()
#set (test_parameters -np 2 "./CGNSReaderTests")
#add_test(NAME cgns-reader COMMAND "mpirun" ${test_parameters})
