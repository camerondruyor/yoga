cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

find_package(tecio QUIET)


set(header_files
        ParfaitViz.h
        ParallelTecplotWriter.h
        ParallelVTKWriter.h
        ParallelDataFrameWriter.h
        )

set(source_files
        ParfaitViz.cpp
        )

add_library(ParfaitViz SHARED ${source_files} ${header_files})
add_library(plugin-parfait::ParfaitViz ALIAS ParfaitViz)
target_include_directories(ParfaitViz PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/plugin-parfait>
        )
target_link_libraries(ParfaitViz PUBLIC
        MessagePasser::MessagePasser
        infinity::infinity
        )

if(TARGET tecio::tecio)
    message("TECIO_ENABLED adding .szplt support")
    set(header_files ${header_files} "SzPltWriter.h")
    target_sources(ParfaitViz PRIVATE SzPltWriter.cpp)
    target_link_libraries(ParfaitViz PUBLIC tecio::tecio)
    target_compile_definitions(ParfaitViz PRIVATE HAVE_HO_TECIO)
    else()
    message("missing TECIO_ENABLED. Disabling .szplt support")
endif()

set_standard_ring_rpath(ParfaitViz)

install(FILES ${header_files}
        DESTINATION include/plugin-parfait/Viz)

install(TARGETS ParfaitViz
        EXPORT ParfaitViz
        LIBRARY DESTINATION lib)

install(EXPORT ParfaitViz
        NAMESPACE plugin-parfait::
        DESTINATION cmake)

    if(FALSE)
set(HO_SRC
        DepsViz.cpp
        )

set(header_files
        ${header_files}
        DepsViz.h
        VtkHacking.h
        ParallelTecplotWriter.h
        )

add_library(DepsViz SHARED ${HO_SRC} ${header_files})
add_library(plugin-parfait::DepsViz ALIAS DepsViz)
target_include_directories(DepsViz
        PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/plugin-parfait>
        PRIVATE
          ${VTK_INCLUDE_DIRS}
        )
target_link_libraries(DepsViz PUBLIC
        ${VTK_LIBRARIES}
        parfait::parfait
        tracer::tracer
        MessagePasser::MessagePasser
        infinity::infinity
        )

set_standard_ring_rpath(DepsViz)

install(FILES ${HEADER_FILES}
        DESTINATION include/plugin-parfait/Viz)

install(TARGETS DepsViz
        EXPORT DepsVizTargets
        LIBRARY DESTINATION lib)

install(EXPORT DepsVizTargets
        NAMESPACE plugin-parfait::
        DESTINATION cmake)
endif()
