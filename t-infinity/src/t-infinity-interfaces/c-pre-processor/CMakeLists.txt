cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(HEADERS
        c_pre_processor.h
		)

set(SOURCES
		c_pre_processor.cpp)


add_library(c_pre_processor SHARED ${SOURCES})
add_library(infinity::c_pre_processor ALIAS c_pre_processor)
target_include_directories(interfaces INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
		)

target_link_libraries(c_pre_processor infinity::infinity)


install(FILES ${HEADERS}
        DESTINATION include/c-pre-processor)

install(TARGETS c_pre_processor
        EXPORT c_pre_processor
        DESTINATION lib
		RUNTIME DESTINATION lib
		)


install(EXPORT c_pre_processor
        NAMESPACE infinity:: DESTINATION cmake)


if(RING_BUILD_TESTING)
	add_catch_unit_test(c_pre_processor_tests c_pre_processor_tests.cpp)
	target_link_libraries(c_pre_processor_tests PRIVATE c_pre_processor)
	target_compile_definitions(c_pre_processor_tests PRIVATE -DGRIDS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../../../grids/")
	mpi_test_requires_grids(c_pre_processor_tests 1)
endif()
