cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(HEADERS
		tinf_mesh.h
        tinf_enum_definitions.h
		tinf_fluid_solver.h
		)


add_library(interfaces INTERFACE)
add_library(infinity::interfaces ALIAS interfaces)
target_include_directories(interfaces INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
		)


install(FILES ${HEADERS}
        DESTINATION include/t-infinity-interfaces)

install(TARGETS interfaces
        EXPORT _infinity_interfaces
        DESTINATION cmake
        )

install(EXPORT _infinity_interfaces
        NAMESPACE infinity:: DESTINATION cmake)

add_subdirectory(c-pre-processor)

if(${PARFAIT_ENABLE_PARMETIS})
    find_package(ParMETIS QUIET)
    if(${ParMETIS_FOUND})
        message("ParMETIS found++! Libs: ${PARMETIS_LIBRARIES}  Path: ${PARMETIS_INCLUDE_DIRS}")
    else()
        message("Could not find ParMETIS.  Continuing without ParMETIS support")
    endif()
endif()

if(ParMETIS_FOUND)
find_package(parfait REQUIRED)
add_subdirectory(c-grouping-pre-processor)
endif()
