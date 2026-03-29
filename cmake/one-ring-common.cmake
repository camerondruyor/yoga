function(set_target_rpath NAME)
    set(extra_paths ${ARGN})

    if (APPLE)
        set(CMAKE_MACOSX_RPATH TRUE)
        set(RPATH_ROOT "@loader_path")
    else ()
        set_target_properties(${NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH FALSE)
        set(RPATH_ROOT "\$ORIGIN")
    endif ()

    if (RING_EXTRA_RPATH)
        list(APPEND target_rpath_directories ${RING_EXTRA_RPATH})
    endif ()

    foreach (path ${ARGN})
        if (IS_ABSOLUTE ${path})
            list(APPEND target_rpath_directories "${path}")
        else ()
            list(APPEND target_rpath_directories "${RPATH_ROOT}/${path}")
        endif ()
    endforeach ()
    list(APPEND target_rpath_directories "${RPATH_ROOT}")

    # Don't skip the full RPATH for the build tree
    set_target_properties(${NAME} PROPERTIES SKIP_BUILD_RPATH FALSE)

    # add the automatically determined parts of the RPATH
    set_target_properties(${NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

    foreach (rpath_link_dir ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES})
        # append RPATH to be used when installing, but only if it's not a system directory
        list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${rpath_link_dir}" isSystemDir)
        if ("${isSystemDir}" STREQUAL "-1")
            list(APPEND target_rpath_directories ${rpath_link_dir})
        endif ("${isSystemDir}" STREQUAL "-1")
    endforeach ()
    set_target_properties(${NAME} PROPERTIES INSTALL_RPATH "${target_rpath_directories}")
endfunction()

function(set_standard_ring_rpath LIBRARY_NAME)
    set_target_rpath(${LIBRARY_NAME} . ../lib ../lib64)
endfunction()

function(add_mpi_test TARGET_NAME NUMBER_OF_MPI_PROCESSES)
    if (RING_BUILD_TESTING)
        set(TEST_NAME ${TARGET_NAME}-${NUMBER_OF_MPI_PROCESSES}-ranks)
        set(test_parameters ${RING_TEST_MPI_ARGUMENTS} -np ${NUMBER_OF_MPI_PROCESSES} "./${TARGET_NAME}")
        add_test(NAME ${TEST_NAME} COMMAND ${MPIEXEC_EXECUTABLE} ${test_parameters})
        set_tests_properties(${TEST_NAME} PROPERTIES PROCESSORS ${NUMBER_OF_MPI_PROCESSES})
    endif ()
endfunction()

function(add_catch_unit_test TARGET_NAME SOURCE_FILES)
    if (RING_BUILD_TESTING)
        find_package(Catch2 3 REQUIRED)
        find_package(CatchTestRunners REQUIRED)
        set(_UNIT_SOURCES ${SOURCE_FILES} ${ARGN})
        add_executable(${TARGET_NAME} ${_UNIT_SOURCES})
        target_link_libraries(${TARGET_NAME} PRIVATE
                Catch2::Catch2
                CatchTestRunners::Runner)
        add_mpi_test(${TARGET_NAME} 1)
    endif ()
endfunction()

function(test_single_omp_thread TEST_NAME)
    set_property(TEST ${TEST_NAME} APPEND PROPERTY ENVIRONMENT OMP_NUM_THREADS=1)
endfunction()

function(exit_if_size_types_invalid)
    include(CheckTypeSize)
    check_type_size("int" INT_SIZE)
    if (NOT ${INT_SIZE} MATCHES "4")
        message(FATAL_ERROR "size of int is not 4 bytes.  This must be a cool system!  Please contanct a T-infinity dev to claim your prize!")
    else ()
        message("int is ${INT_SIZE} bytes long")
    endif ()

    check_type_size("long" LONG_SIZE)
    if (NOT ${LONG_SIZE} MATCHES "8")
        message(FATAL_ERROR "size of long is not 8 bytes.  This must be a cool system!  Please contanct a T-infinity dev to claim your prize!")
    else ()
        message("long is ${LONG_SIZE} bytes long")
    endif ()
endfunction()

macro(setOpenMPTarget)
    find_package(OpenMP ${ARGN})
    if (OpenMP_FOUND)
        # For CMake < 3.9, we need to make the target ourselves
        if (NOT TARGET OpenMP::OpenMP_CXX)
            find_package(Threads REQUIRED)
            add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
            set_property(TARGET OpenMP::OpenMP_CXX
                    PROPERTY INTERFACE_COMPILE_OPTIONS ${OpenMP_CXX_FLAGS})
            # Only works if the same flag is passed to the linker; use CMake 3.9+ otherwise (Intel, AppleClang)
            set_property(TARGET OpenMP::OpenMP_CXX
                    PROPERTY INTERFACE_LINK_LIBRARIES ${OpenMP_CXX_FLAGS} Threads::Threads)

        endif ()
    endif ()
endmacro()

function(target_use_install_rpath NAME)
    set_target_properties(${NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
endfunction()

function(add_subcommand prefix extension_name command_name SOURCE_FILES)
    set(REAL_TARGET "${prefix}_SubCommand_${extension_name}_${command_name}")
    set(_UNIT_SOURCES ${SOURCE_FILES} ${ARGN})
    add_library(${REAL_TARGET} MODULE ${_UNIT_SOURCES})
    #add_library(${target_name} ALIAS ${REAL_TARGET})
    target_compile_definitions(${REAL_TARGET} PRIVATE DRIVER_PREFIX="${prefix}")
    target_link_libraries(${REAL_TARGET} PUBLIC infinity::infinity)
    set_target_rpath(${REAL_TARGET} ../lib)
    target_use_install_rpath(${REAL_TARGET})
    install(TARGETS ${REAL_TARGET} EXPORT infinity-targets DESTINATION lib)
endfunction()

function(subcommand_link_libraries prefix extension_name command_name LIBRARIES)
    set(REAL_TARGET "${prefix}_SubCommand_${extension_name}_${command_name}")
    set(_UNIT_LIBRARIES ${LIBRARIES} ${ARGN})
    target_link_libraries(${REAL_TARGET} PUBLIC ${_UNIT_LIBRARIES})
endfunction()

function(subcommand_compile_definitions target_name _DEFINITIONS)
    set(REAL_TARGET "SubCommand_${target_name}")
    target_compile_definitions(${REAL_TARGET} PRIVATE "${_DEFINITIONS}")
endfunction()

function(test_requires_grids TEST_NAME)
    set_tests_properties(${TEST_NAME} PROPERTIES
            REQUIRED_FILES "${CMAKE_SOURCE_DIR}/grids"
            )
endfunction()

function(mpi_test_requires_grids TEST_NAME NUM_RANKS)
    test_requires_grids(${TEST_NAME}-${NUM_RANKS}-ranks)
endfunction()

function(ring_precompiled_headers TARGET_NAME)
    if (RING_ENABLE_PRECOMPILED_HEADERS)
        set(pch_options ${ARGN})
        target_precompile_headers(${TARGET_NAME} ${pch_options})
    endif ()
endfunction()

function(ring_install_executable)
    if (NOT RING_DISABLE_EXECUTABLE_INSTALL)
        install(${ARGN})
    endif ()
endfunction()
