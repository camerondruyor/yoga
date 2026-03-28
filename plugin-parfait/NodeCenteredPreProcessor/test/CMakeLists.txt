cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(TEST_SOURCES
        InferCellOwnersTests.cpp
        DistributorTests.cpp
        DetermineNodeOwnersFromRange.cpp
        )

add_catch_unit_test(NC_Tests ${TEST_SOURCES})
add_mpi_test(NC_Tests 2)
mpi_test_requires_grids(NC_Tests 1)
mpi_test_requires_grids(NC_Tests 2)
target_include_directories(NC_Tests PUBLIC
        . ../
        )
target_link_libraries(NC_Tests PUBLIC
        NC_PreProcessor
        )
target_compile_definitions(NC_Tests PRIVATE -DGRIDS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../../grids/")
