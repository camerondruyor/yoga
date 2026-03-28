cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# required for IN_LIST on "old" versions of cmake
cmake_policy(SET CMP0057 NEW)
set(CMAKE_INSTALL_MESSAGE LAZY)

# to eliminate MPT warning about MPI_ROOT being set
if (${CMAKE_VERSION} VERSION_LESS 3.10)
    message("Cannot use CMake policy CMP0074, expect MPI_ROOT warnings")
    message("Please consider updating to CMake 3.10 or greater")
    message("This will be a requirement in a future version")
else ()
    cmake_policy(SET CMP0074 NEW)
endif ()

project(one-ring
        VERSION 1.0.0
        LANGUAGES C CXX)

# require strict conformance to C++ standard by default (cmake default is the opposite)
set(CMAKE_CXX_EXTENSIONS OFF)

function(ring_option NAME MESSAGE DEVELOP_DEFAULT DISTRIBUTION_DEFAULT)
    #    Check if this a distribution (missing go.py) to determine option default
    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/go.py)
        option(${NAME} ${MESSAGE} ${DEVELOP_DEFAULT})
    else ()
        option(${NAME} ${MESSAGE} ${DISTRIBUTION_DEFAULT})
    endif ()
endfunction()

set(RING_MAX_ATTEMPTED_CXX "17" CACHE STRING "Maximum CXX to automatically detect")
ring_option(RING_DEVELOPER_COMPILE_MODE "Compile Using Developer Settings" ON OFF)
ring_option(RING_BUILD_TESTING "Build one-ring tests" ON OFF)
ring_option(BUILD_SHARED_LIBS "Build one-ring shared libraries" ON ON)
ring_option(BUILD_PYTHON_RUNTIME "Build one-ring python tools" ON OFF)
ring_option(RING_ENABLE_CUDA "Enable CUDA support in Kokkos" OFF OFF)
ring_option(RING_ENABLE_POSIX_HINTS "Enable POSIX hints" ON ON)
ring_option(RING_KOKKOS_DEFAULTS "Set default flags for Kokkos" ON ON)
ring_option(RING_DISABLE_KOKKOS "Force no Kokkos dependencies to be built" OFF OFF)
ring_option(RING_ENABLE_CCACHE "Enable ccache for faster recompilation" OFF OFF)
ring_option(RING_TIME_COMPILE "Time compilation of each file" OFF OFF)
ring_option(RING_INSTALL_DEV_TOOLS "Install Developer Tools" ON OFF)
ring_option(RING_CHECK_INTEL_GCC "Require a compatible version of gcc with intel compiler" ON ON)
ring_option(RING_ENABLE_CPACK "Enable CPack support for building distributions" OFF OFF)
ring_option(RING_ENABLE_PRECOMPILED_HEADERS "Enable precompiled_header support" OFF OFF)
ring_option(RING_DISABLE_EXECUTABLE_INSTALL "Disable installing executables built by one-ring subpackages" OFF OFF)
ring_option(RING_BUILD_GUI "Build one-ring gui" OFF OFF)

# turn off the c++ mpi bindings that no one uses
set(MPI_CXX_SKIP_MPICXX
        TRUE
        CACHE BOOL "")
# also turn them off for MPT, extra specially
add_definitions(-DMPI_NO_CPPBIND)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel;asan;tsan" CACHE STRING "" FORCE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
if (${RING_DEVELOPER_COMPILE_MODE})
    message("Compile mode: Development")
else ()
    message("Compile mode: Production")
endif ()
include(DetectCXXStandard)
include(one-ring-common)
include(GitVersion)
include(KokkosOptions)
include(RingBuildTypes)

# Require C++ 17 when compiling for cuda
if (RING_ENABLE_CUDA)
    enable_language(CUDA)
    if (NOT "${RING_CXX_STANDARD}" STREQUAL "17")
        message(FATAL_ERROR "Cuda support requested, but compiler does not support C++17")
    endif ()
endif ()

message(STATUS "Git version : ${GIT_DATE}-${GIT_COMMIT_SHA}")

if (${RING_CHECK_INTEL_GCC})
    include(IntelCompilerSettings)
endif ()

if (${RING_ENABLE_PRECOMPILED_HEADERS})
    if (${CMAKE_VERSION} VERSION_LESS "3.16.0")
        message(WARNING "RING_ENABLE_PCH requires to CMake >= 3.16.0, current version: ${CMAKE_VERSION}.  Precompiled header support is disabled")
        set(RING_ENABLE_PRECOMPILED_HEADERS OFF CACHE BOOL "Disable pch support" FORCE)
    endif ()
endif ()

# To avoid every cmake package looking for the same package over and over, find
# them once here
find_package(Git QUIET)
find_package(MPI QUIET)
find_package(VTK QUIET)
setOpenMPTarget()

set(excluded_directories "")
list(APPEND excluded_directories "cmake")

set(all_packages "")
set(tpl_packages "")
if (${CMAKE_VERSION} VERSION_LESS "3.12.0")
    file(GLOB subproject_directories */CMakeLists.txt)
    file(GLOB tpl_directories third-party-libraries/*/CMakeLists.txt)
else ()
    file(GLOB subproject_directories CONFIGURE_DEPENDS */CMakeLists.txt)
    file(GLOB tpl_directories CONFIGURE_DEPENDS third-party-libraries/*/CMakeLists.txt)
endif ()
foreach (subproject ${subproject_directories})
    get_filename_component(subproject "${subproject}" DIRECTORY)
    get_filename_component(subproject "${subproject}" NAME)
    if (NOT ${subproject} IN_LIST excluded_directories)
        list(APPEND all_packages ${subproject})
    endif ()
endforeach ()
foreach (subproject ${tpl_directories})
    get_filename_component(subproject "${subproject}" DIRECTORY)
    get_filename_component(subproject "${subproject}" NAME)
    if (NOT ${subproject} IN_LIST excluded_directories)
        list(APPEND tpl_packages ${subproject})
    endif ()
endforeach ()
set(all_packages ${tpl_packages} ${all_packages})

if (NOT only)
    set(packages_to_build ${all_packages} CACHE INTERNAL "" FORCE)
else ()
    set(packages_to_build ${only} CACHE INTERNAL "" FORCE)
endif ()
mark_as_advanced(packages_to_build)

set(find_once MPI VTK OpenMP)
set(packages_that_need_kokkos Kokkos Kokkos-simd Trilinos ra enigma voithos loki hypersolve jupiter)
set(packages_that_need_cpp17 octmesh)

if (APPLE)
    if (${RING_ENABLE_POSIX_HINTS})
        message("Apple system detected. Disabling RING_ENABLE_POSIX_HINTS.")
    endif ()
    set(RING_ENABLE_POSIX_HINTS false)
endif ()

if (RING_DISABLE_KOKKOS)
    message("disabling Kokkos/Trilinos usage in one-ring")
    message("disabling Kokkos-dependent packages: ${packages_that_need_kokkos}")
    set(packages_to_skip ${packages_to_skip} "${packages_that_need_kokkos}")
else ()
    find_package(Trilinos QUIET)
    list(APPEND find_once Trilinos)
endif ()

if(NOT BUILD_PYTHON_RUNTIME)
    set(packages_to_skip ${packages_to_skip}, "pybind11")
endif()
if(NOT RING_BUILD_GUI)
    set(packages_to_skip ${packages_to_skip} "tui")
endif()

if(NOT BUILD_PYTHON_RUNTIME)
    set(packages_to_skip ${packages_to_skip}, "pybind11")
endif()

if (NOT "${RING_CXX_STANDARD}" STREQUAL "17")
    set(packages_to_skip ${packages_to_skip} "${packages_that_need_cpp17}")
endif()

# skip packages
if(packages_to_skip)
    SET(remaining_packages ${packages_to_build})
    list(REMOVE_ITEM remaining_packages ${packages_to_skip})
    set(packages_to_build ${remaining_packages} CACHE INTERNAL "" FORCE)
    foreach (p ${packages_to_skip})
        set(CMAKE_DISABLE_FIND_PACKAGE_${p} true CACHE BOOL "" FORCE)
    endforeach ()
endif()


if (RING_ENABLE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if (CCACHE_PROGRAM)
        message("ccache found: ${CCACHE_PROGRAM}")
        message("building with ccache support")
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    else ()
        message(FATAL_ERROR "ccache requested but not found.")
    endif ()
endif ()

if (RING_TIME_COMPILE)
    message("-- Compilation timing enabled")
    set(RING_TIMER ${CMAKE_BINARY_DIR}/timer.sh)
    set(TIMING_OUTPUT_PATH ${CMAKE_BINARY_DIR}/compile_timings)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/ringtools/timer.sh.in ${RING_TIMER} @ONLY)
    set(CMAKE_C_COMPILER_LAUNCHER "${RING_TIMER}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${RING_TIMER}")
    if (RING_ENABLE_CCACHE)
        set(CMAKE_C_COMPILER_LAUNCHER ${RING_TIMER} ${CCACHE_PROGRAM})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${RING_TIMER} ${CCACHE_PROGRAM})
    endif ()
endif ()

# If one-ring is the top level and testing in enabled, included CTest
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND RING_BUILD_TESTING)
    include(CTest)
    enable_testing()
endif ()

# Cannot built kokkos twice (once here, once in Trilinos)
if (Trilinos_FOUND)
    message("Trilinos found - Disable Kokkos as subdirectory")
    find_package(Kokkos QUIET)
    if (Kokkos_FOUND)
        list(REMOVE_ITEM packages_to_build Kokkos)
        list(APPEND find_once Kokkos)
        set(RING_KOKKOS_DEFAULTS
                false
                CACHE BOOL "" FORCE)
    endif ()
endif ()

# Disable building Catch2-related packages when tests disable, just to be safe...
if (NOT RING_BUILD_TESTING)
    list(REMOVE_ITEM packages_to_build Catch2)
    list(REMOVE_ITEM packages_to_build CatchTestRunners)
endif ()

macro(message)
    if (${ARGC} EQUAL 1)
        _message(STATUS "[${PROJECT_NAME}] ${ARGV0}")
    elseif (${ARGC} EQUAL 2)
        _message(${ARGV0} "[${PROJECT_NAME}] ${ARGV1}")
    endif ()
endmacro()

set(one_ring_binary_dir "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "build directory of one-ring" FORCE)
set(one_ring_source_dir "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "root directory of one-ring" FORCE)

macro(find_package)
    if (CMAKE_DISABLE_FIND_PACKAGE_${ARGV0})
        # Redirect disabled packages to standard cmake find_package,
        # so that error messages are correctly reported.
        _find_package(${ARGV})
    elseif ("${ARGV0}" IN_LIST all_packages)
        # Treat these packages as subdirectories
        if (NOT "${ARGV0}" IN_LIST packages_to_build)
            if (${ARGV0} IN_LIST tpl_packages)
                add_subdirectory("${one_ring_source_dir}/third-party-libraries/${ARGV0}" "${one_ring_binary_dir}/third-party-libraries/${ARGV0}")
            else()
                add_subdirectory("${one_ring_source_dir}/${ARGV0}" "${one_ring_binary_dir}/${ARGV0}")
            endif()
            set(${ARGV0}_FOUND TRUE)
            set(packages_to_build "${packages_to_build};${ARGV0}" CACHE INTERNAL "" FORCE)
        endif ()

    elseif ("${ARGV0}" IN_LIST find_once)
        # Do not find these packages more than once
    else ()
        _find_package(${ARGV})
    endif ()
endmacro()

if (RING_KOKKOS_DEFAULTS)
    message("Setting one-ring kokkos defaults")
    set_one_ring_kokkos_defaults()
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

foreach (ONE_RING_PACKAGE ${packages_to_build})
    set(${ONE_RING_PACKAGE}_FOUND TRUE)
endforeach ()

foreach (ONE_RING_PACKAGE ${packages_to_build})
    if (${ONE_RING_PACKAGE} IN_LIST tpl_packages)
        add_subdirectory(third-party-libraries/${ONE_RING_PACKAGE})
    else()
        add_subdirectory(${ONE_RING_PACKAGE})
    endif()
endforeach ()

list(SORT packages_to_build)

if (RING_BUILD_TESTING AND NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/grids)
    message(WARNING "The ${CMAKE_CURRENT_LIST_DIR}/grids directory is missing, but required for some tests. Try running ${CMAKE_CURRENT_LIST_DIR}/get_grids.sh to populate that directory before running tests.")
endif ()

if (RING_ENABLE_CPACK)
    message("CPack support enabled.  Building source file distribution list.")
    file(GLOB IGNORE_LIST * LIST_DIRECTORIES true)
    set(distribution_directories ${packages_to_build} cmake)
    if ("Kokkos" IN_LIST find_once)
        list(APPEND distribution_directories Kokkos)
    endif ()

    # The 2-level hierachy makes this part necessary.
    # Otherwise, the TPL packages (e.g., Kokkos, nanoflann, etc.) won't get packaged.
    list(REMOVE_ITEM IGNORE_LIST ${CMAKE_CURRENT_LIST_DIR}/third-party-libraries)
    foreach (TPL_PACKAGE ${tpl_packages})
        list(APPEND IGNORE_LIST ${CMAKE_CURRENT_LIST_DIR}/third-party-libraries/${TPL_PACKAGE})
    endforeach()

    execute_process(
            COMMAND ./ringtools/make_version_file.sh
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )
    execute_process(
            COMMAND ./ringtools/make_readme_file.sh ${distribution_directories}
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )
    foreach (ONE_RING_PACKAGE ${distribution_directories})
        if (${ONE_RING_PACKAGE} IN_LIST tpl_packages)
            set(ONE_RING_PACKAGE third-party-libraries/${ONE_RING_PACKAGE})
        endif()
        list(REMOVE_ITEM IGNORE_LIST ${CMAKE_CURRENT_LIST_DIR}/${ONE_RING_PACKAGE})
        if (Git_FOUND AND EXISTS ${PROJECT_SOURCE_DIR}/.git)
            execute_process(
                    COMMAND ${GIT_EXECUTABLE} ls-files
                    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${ONE_RING_PACKAGE}
                    OUTPUT_VARIABLE GIT_TRACKED_FILES
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
            file(GLOB_RECURSE package_files ${ONE_RING_PACKAGE}/* LIST_DIRECTORIES true)
            string(REGEX REPLACE "\n" ";" GIT_TRACKED_FILES "${GIT_TRACKED_FILES}")
            foreach (SOURCE ${GIT_TRACKED_FILES})
                list(REMOVE_ITEM package_files "${CMAKE_CURRENT_LIST_DIR}/${ONE_RING_PACKAGE}/${SOURCE}")
            endforeach ()
            list(APPEND IGNORE_LIST ${package_files})
        else ()
            message(WARNING "Git not available.  CPack may package more source code than intended.")
        endif ()
    endforeach ()
    list(REMOVE_ITEM IGNORE_LIST ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt)
    list(REMOVE_ITEM IGNORE_LIST ${CMAKE_CURRENT_LIST_DIR}/VERSION)
    list(REMOVE_ITEM IGNORE_LIST ${CMAKE_CURRENT_LIST_DIR}/README)
    list(REMOVE_ITEM IGNORE_LIST ${CMAKE_CURRENT_LIST_DIR}/configure)
    list(REMOVE_ITEM IGNORE_LIST ${CMAKE_CURRENT_LIST_DIR}/.configure-custom.sh)
    set(CPACK_PACKAGE_VENDOR "NASA Langley Research Center")
    set(CPACK_PACKAGE_NAME "T-Infinity")
    string(TIMESTAMP PACKAGE_DATE "%Y-%m-%d")
    set(CPACK_PACKAGE_VERSION ${PACKAGE_DATE})
    set(CPACK_SOURCE_IGNORE_FILES ${IGNORE_LIST})
    set(CPACK_SOURCE_GENERATOR "TGZ")
    include(CPack)
    message("Packaging the following one-ring packages:")
    foreach (package_name ${packages_to_build})
        message("  ${package_name}")
    endforeach ()
else ()
    message("Building the following one-ring packages: ${packages_to_build}")
endif ()


if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(cmake)
endif ()
