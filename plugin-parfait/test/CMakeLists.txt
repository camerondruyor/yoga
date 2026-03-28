cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(TEST_SOURCES
        CellRedistributorTests.cpp
        InitialNodeSyncerTest.cpp
        NoDepsVTKTests.cpp
        UgridReaderTests.cpp
        GmshReaderTests.cpp
        GmshWriterTests.cpp
        ParallelTecplotBinaryWriterTests.cpp
        SerialImporterTests.cpp
        SettingsTests.cpp
        SU2IOTests.cpp
        STLReaderTests.cpp
        VTKReaderTests.cpp
        )

if(TARGET tecio::tecio)
    set(TEST_SOURCES ${TEST_SOURCES} "TecioWriterTests.cpp")
endif()

add_catch_unit_test(ParfaitPluginsTests ${TEST_SOURCES})
add_mpi_test(ParfaitPluginsTests 2)
mpi_test_requires_grids(ParfaitPluginsTests 1)
mpi_test_requires_grids(ParfaitPluginsTests 2)
target_include_directories(ParfaitPluginsTests PUBLIC
        ../shared
        )
target_link_libraries(ParfaitPluginsTests PUBLIC
        NC_PreProcessor
        tracer::tracer
        plugin-parfait::ParfaitViz
        )

target_compile_definitions(ParfaitPluginsTests PRIVATE -DGRIDS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../grids/")
