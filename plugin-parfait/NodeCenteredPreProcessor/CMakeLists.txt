cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(HEADER_FILES
        CellRedistributor.h
        Distributor.h
        GraphBuilder.h
        InitialNodeSyncer.h
        NaiveMesh.h
        NC_PartitionerBase.h
        NC_PreProcessor.h
        NC_RCBPartitioner.h
        PProcessor.h
        PartitionGrouper.h
        Redistributor.h
        )

set(SRC
        CellRedistributor.cpp
        GraphBuilder.cpp
        InitialNodeSyncer.cpp
        NaiveMesh.cpp
        NC_PreProcessor.cpp
        NC_PartitionerBase.cpp
        Redistributor.cpp
        )

set(HEADER_FILES_GRP
        CellRedistributor.h
        Distributor.h
        GraphBuilder.h
        InitialNodeSyncer.h
        NaiveMesh.h
        NC_PartitionerBase.h
        NC_GrpPreProcessor.h
        NC_RCBPartitioner.h
        PProcessor.h
        PartitionGrouper.h
        Redistributor.h
        )

set(SRC_GRP
        CellRedistributor.cpp
        GraphBuilder.cpp
        InitialNodeSyncer.cpp
        NaiveMesh.cpp
        NC_GrpPreProcessor.cpp
        NC_PartitionerBase.cpp
        PartitionGrouper.cpp
        Redistributor.cpp
        )

add_library(NC_PreProcessor SHARED ${SRC} ${HEADER_FILES})
add_library(parfait::NC_PreProcessor ALIAS NC_PreProcessor)
target_include_directories(NC_PreProcessor PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        )

target_link_libraries(NC_PreProcessor PUBLIC
        tracer::tracer
        parfait::parfait
        infinity::infinity
        plugin_parfait_core
        parfait::S_PreProcessor
        Threads::Threads
        )

if(ParMETIS_FOUND)
    target_sources(NC_PreProcessor PRIVATE NC_ParmetisPartitioner.h)
    set(HEADER_FILES ${HEADER_FILES} "NC_ParmetisPartitioner.h")
    target_link_libraries(NC_PreProcessor PUBLIC ParMETIS::ParMETIS )
    target_compile_definitions(NC_PreProcessor PRIVATE HAVE_PARMETIS)
endif()

if(ParMETIS_FOUND)
    add_library(PartitionGrouper SHARED PartitionGrouper.h PartitionGrouper.cpp)
    target_link_libraries(PartitionGrouper PUBLIC
        infinity::infinity
        plugin_parfait_core
        ParMETIS::ParMETIS )
    target_compile_definitions(PartitionGrouper PRIVATE HAVE_PARMETIS)
    target_include_directories(PartitionGrouper PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        )

    install(TARGETS PartitionGrouper
            EXPORT PartitionGrouper
            LIBRARY DESTINATION lib)

     install(EXPORT PartitionGrouper
             NAMESPACE parfait::
             DESTINATION cmake)

add_library(NC_GrpPreProcessor SHARED ${SRC_GRP} ${HEADER_FILES_GRP})
add_library(parfait::NC_GrpPreProcessor ALIAS NC_GrpPreProcessor)
target_include_directories(NC_GrpPreProcessor PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        )

target_link_libraries(NC_GrpPreProcessor PUBLIC
        tracer::tracer
        parfait::parfait
        infinity::infinity
        plugin_parfait_core
        parfait::S_PreProcessor
        Threads::Threads
        ParMETIS::ParMETIS
        )
target_compile_definitions(NC_GrpPreProcessor PRIVATE HAVE_PARMETIS)

install(TARGETS NC_GrpPreProcessor
        EXPORT NC_GrpPreProcessor
        LIBRARY DESTINATION lib)

install(EXPORT NC_GrpPreProcessor
        NAMESPACE parfait::
        DESTINATION cmake)

endif()

set_standard_ring_rpath(NC_PreProcessor)

install(FILES ${HEADER_FILES} DESTINATION include/NodeCenteredPreProcessor)

install(TARGETS NC_PreProcessor
    EXPORT NC_PreProcessor
    LIBRARY DESTINATION lib)

install(EXPORT NC_PreProcessor
        NAMESPACE parfait::
        DESTINATION cmake)

add_subcommand(inf core examine ExamineCommand.cpp)
subcommand_link_libraries(inf core examine
        plugin_parfait_core
        parfait::parfait)

if(RING_BUILD_TESTING)
    add_subdirectory(test)
endif()
