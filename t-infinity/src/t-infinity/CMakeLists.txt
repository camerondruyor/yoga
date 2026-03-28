cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

get_git_version(GIT_DATE GIT_COMMIT_SHA)
set(T_INFINITY_VERSION ${GIT_DATE}-${GIT_COMMIT_SHA})
configure_file(Version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/Version.cpp @ONLY)

set(SOURCES
        AddHalo.cpp
        AddMissingFaces.cpp
        BandwidthReducer.cpp
        BetterDistanceCalculator.cpp
        CartMesh.cpp
        Cell.cpp
        CellIdFilter.cpp
        CellSelectedMesh.cpp
        Communicator.cpp
        CommonSubcommandFunctions.cpp
        CompositeMeshBuilder.cpp
        Demangle.cpp
        DistanceCalculator.cpp
        ElevateMesh.cpp
        Extract.cpp
        FaceNeighbors.cpp
        FieldTools.cpp
        FluidSolverTools.cpp
        FieldShuffle.cpp
        FileStreamer.cpp
        GhostSyncer.cpp
        GlobalPairIds.cpp
        GlobalToLocal.cpp
        HangingEdgeRemesher.cpp
        Hiro.cpp
        IsoSampling.cpp
        LineSampling.cpp
        MeasureLoadBalance.cpp
        MeshBuilder.cpp
        MeshConnectivity.cpp
        MeshFixer.cpp
        MeshExploder.cpp
        MeshExtruder.cpp
        MeshHelpers.cpp
        MeshMechanics.cpp
        MeshMemoryConsumption.h
        MeshMirror.cpp
        MeshOptimization.cpp
        MeshQualityMetrics.cpp
        MeshSanityChecker.cpp
        MeshShuffle.cpp
        MeshSubdomain.cpp
        MetricManipulator.cpp
        QuiltTags.cpp
        PancakeMeshAdapterFrom.h
        ParallelUgridExporter.cpp
        Periodicity.cpp
        PluginLocator.cpp.in
        PartitionedStructuredMesh.cpp
        ReorderMesh.cpp
        ReverseCuthillMckee.cpp
        SelectedField.cpp
        SliverCellDetection.cpp
        Shortcuts.cpp
        Snap.cpp
        SurfaceSmoothness.h
        SurfaceMesh.cpp
        PancakeMeshAdapterTo.cpp
        PartitionDiffusion.cpp
        ScriptVisualization.cpp
        SetNodeOwners.cpp
        StructuredBCRegion.cpp
        StructuredBlockConnectivity.cpp
        StructuredMeshGradients.cpp
        StructuredMeshPlot.cpp
        StructuredMeshSequencing.cpp
        StructuredMeshGlobalIds.cpp
        StructuredMeshConnectivity.cpp
        StructuredMeshHelpers.cpp
        StructuredMeshGlobalIds.cpp
        StructuredMeshShuffle.cpp
        StructuredMeshValues.h
        SurfaceEdgeNeighbors.cpp
        SurfaceMeshReconstruction.h
        ${CMAKE_CURRENT_BINARY_DIR}/Version.cpp
        StructuredMeshPartitioner.cpp
        SplitStructuredBlockMapping.cpp
        StructuredToUnstructuredMeshAdapter.cpp
        StructuredTinfMesh.cpp
        SurfaceElementWinding.cpp
        SurfaceNeighbors.cpp
        TopologicalRotation.cpp
        TinfMesh.cpp
        TinfMeshAppender.cpp
        VectorFieldAdapter.cpp
        VisualizePartition.cpp
        VizFromDictionary.cpp
        Wiggle.h
        )

set(HEADERS
        AddHalo.h
        AddMissingFaces.h
        BandwidthReducer.h
        BetterDistanceCalculator.h
        BoundaryNodes.h
        CartMesh.h
        Cell.h
        CellIdFilter.h
        CellSelectedMesh.h
        CellSelector.h
        CommonAliases.h
        CommonSubcommandFunctions.h
        Communicator.h
        CSolutionFieldExtractor.h
        CommandRunner.h
        CompositeMeshBuilder.h
        Debug.h
        Demangle.h
        ElevateMesh.h
        FixedPointHessianCombiner.h
        MapbcLumper.h
        MeshInquisitor.h
        MeshExtruder.h
        MeshSpacing.h
        MotionMatrixParser.h
        DistanceCalculator.h
        DivineLoadBalancer.h
        Emeril.h
        Extract.h
        ExtentIntersections.h
        ExtensionsHelpers.h
        ExtensionsCommand.h
        FaceNeighbors.h
        FieldExtractor.h
        FieldLoader.h
        FieldTools.h
        FieldStatistics.h
        FileStreamer.h
        FilterFactory.h
        FilterTools.h
        FieldShuffle.h
        FluidSolver2DWrapper.h
        FluidSolverTools.h
        Geometry.h
        GhostSyncer.h
        GlobalToLocal.h
        GlobalPairIds.h
        Gradients.h
        HangingEdgeRemesher.h
        Hiro.h
        InfinityToVtk.h
        InfinityToAFLR.h
        InternalFaceRemover.h
        IsoSampling.h
        LayerInsertion.h
        LoadBalancer.h
        Mangle.h
        MapBcFileCombiner.h
        MeshBuilder.h
        MeshMover.h
        MetricHelpers.h
        MetricManipulator.h
        NormalGradientPointGeneration.h
        PancakeMeshAdapterTo.h
        PancakeMeshAdapterFrom.h
        PartitionConnectivityMapper.h
        PartitionDiffusion.h
        PointCloudInterpolator.h
        Periodicity.h
        MapBetweenMeshes.h
        MDVector.h
        MeasureLoadBalance.h
        MeshBuilder.h
        MeshConnectivity.h
        MeshExtents.h
        MeshExploder.h
        MeshSubdomain.h
        MetricAugmentationFactory.h
        MeshExamine.h
        MeshFixer.h
        MeshGatherer.h
        MeshLoader.h
        MeshMirror.cpp
        MeshOptimization.h
        MeshHelpers.h
        MeshQualityMetrics.h
        MeshSanityChecker.h
        MeshShuffle.h
        MeshConnectivity.h
        MeshPrinter.h
        NullCommand.h
        ParallelUgridExporter.h
        PartitionedStructuredMesh.h
        Pipeline.h
        PluginLocator.h
        PluginLoader.h
        QuiltTags.h
        RecipeGenerator.h
        ReorderMesh.h
        ReverseCutthillMckee.h
        ScriptVisualization.h
        SelectedField.h
        SetNodeOwners.h
        SMesh.h
        SliverCellDetection.h
        Shortcuts.h
        Snap.h
        StitchMesh.h
        StructuredBlockConnectivity.h
        StructuredBCRegion.h
        StructuredMeshPlot.h
        StructuredMeshGradients.h
        StructuredToUnstructuredMeshAdapter.h
        StructuredMeshConnectivity.h
        StructuredBlockPlane.h
        StructuredBlockPlaneSyncer.h
        StructuredMeshHelpers.h
        StructuredMeshSyncer.h
        StructuredMeshGlobalIds.h
        StructuredMeshShuffle.h
        StructuredTinfMesh.h
        StructuredSubMesh.h
        StructuredNodeRangeSelector.h
        StructuredMeshFilter.h
        StructuredMeshLoadBalancing.h
        StructuredMeshSequencing.h
        StructuredBlockFace.h
        STLConverters.h
        SubCommand.h
        SubCommandMain.h
        Subdomain.h
        SurfaceMesh.h
        SurfacePlaneSampling.h
        StructuredMeshPartitioner.h
        SplitStructuredBlockMapping.h
        SurfaceNeighbors.h
        SurfaceElementWinding.h
        SurfaceEdgeNeighbors.h
        TetGenWriter.h
        TinfMesh.h
        TinfMeshAppender.h
        WriteUgrid.h
        TopologicalRotation.h
        Version.h
        VectorFieldAdapter.h
        VisualizePartition.h
        VizFactory.h
        VizFromDictionary.h
        )


set(INTERFACE_HEADERS
        ByteStreamInterface.h
        ByteStreamLoaderInterface.h
        DistanceCalculatorInterface.h
        DomainAssemblerInterface.h
        FieldInterface.h
        FluidSolverInterface.h
        GeometryAssociationInterface.h
        GeometryAssociationLoaderInterface.h
        InterpolationInterface.h
        MeshInterface.h
        MeshShard.h
        MeshDeformerInterface.h
        MeshAdaptionInterface.h
        MeshExtruderInterface.h
        MetricCalculatorInterface.h
        RepartitionerInterface.h
        StructuralSolverInterface.h
        StructuredMesh.h
        StructuredMeshLoader.h
        StructuredMeshAdaptationInterface.h
        VizInterface.h
        )

# bake the plugin location in directly, to avoid issues with nvcc
set(TINF_PLUGIN_DIR "${CMAKE_INSTALL_PREFIX}/lib")
configure_file(PluginLocator.cpp.in PluginLocator.cpp)
set(SOURCES ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/PluginLocator.cpp)

add_library(base INTERFACE)
add_library(infinity::base ALIAS base)
target_include_directories(base INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
        ${MPI_INCLUDE_PATH}
        )
target_link_libraries(base INTERFACE ${MPI_LIBRARIES})

# Prevent the clang compiler from issuing warnings about plugin factories
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(base INTERFACE -Wno-return-type-c-linkage)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_options(base INTERFACE -Wno-return-type-c-linkage)
endif ()

install(FILES ${INTERFACE_HEADERS} DESTINATION include/t-infinity)

install(TARGETS base
        EXPORT _infinity_base
        DESTINATION lib
        RUNTIME DESTINATION lib
        )

install(EXPORT _infinity_base
        NAMESPACE infinity:: DESTINATION cmake)

# This is so we don't compile all of t-infinity-infinity twice.  Once for static and once for dynamic.
# We can clean this up if we require cmake 3.12 since that allows target_link_libraries for obj libs.
add_library(infinity_obj OBJECT ${SOURCES} ${HEADERS})
set_target_properties(infinity_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(${RING_ENABLE_POSIX_HINTS})
  target_compile_definitions(infinity_obj PUBLIC ENABLE_POSIX_HINTS)
endif()
target_compile_definitions(infinity_obj PRIVATE ALWAYS_MANGLE)
target_include_directories(infinity_obj PUBLIC
        $<TARGET_PROPERTY:parfait::parfait,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:MessagePasser::MessagePasser,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:tracer::tracer,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:infinity::base,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:infinity::interfaces,INTERFACE_INCLUDE_DIRECTORIES>
        )

ring_precompiled_headers(infinity_obj
        PRIVATE
        <parfait/StringTools.h>
        <parfait/ToString.h>
        <Tracer.h>
        TinfMesh.h
        )

add_library(infinity SHARED $<TARGET_OBJECTS:infinity_obj>)
add_library(infinity::infinity ALIAS infinity)
set_target_properties(infinity PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(infinity PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
        )
target_link_libraries(infinity
        PUBLIC
        ${CMAKE_DL_LIBS}
        parfait::parfait
        MessagePasser::MessagePasser
        tracer::tracer
        infinity::base
        infinity::interfaces
        )

add_library(infinity_static STATIC $<TARGET_OBJECTS:infinity_obj>)
add_library(infinity::infinity_static ALIAS infinity_static)
set_target_properties(infinity_static PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(infinity_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
        )
target_link_libraries(infinity_static
        PUBLIC
        ${CMAKE_DL_LIBS}
        parfait::parfait
        MessagePasser::MessagePasser
        tracer::tracer_static
        infinity::base
        infinity::interfaces
        )

if (BUILD_PYTHON_RUNTIME)
    add_subdirectory(python-bindings)
endif ()

set_standard_ring_rpath(infinity)
#set_standard_ring_rpath(infinity_static)

install(FILES ${HEADERS} DESTINATION include/t-infinity)

install(TARGETS infinity
        EXPORT _infinity_infinity
        DESTINATION lib
        RUNTIME DESTINATION lib
        )

install(TARGETS infinity_static
        EXPORT _infinity_infinity_static
        DESTINATION lib
        ARCHIVE DESTINATION lib
        )

install(EXPORT _infinity_infinity
        NAMESPACE infinity:: DESTINATION cmake)
install(EXPORT _infinity_infinity_static
        NAMESPACE infinity:: DESTINATION cmake)

