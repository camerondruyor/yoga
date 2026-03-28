cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

PROJECT(MessagePasser)
set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake")
include(one-ring-common)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND RING_BUILD_TESTING)
    include(CTest)
    enable_testing()
endif()

file(GLOB_RECURSE MESSAGE_PASSER_HEADERS
        *.h
        *.hpp
        )

find_package(MPI REQUIRED)

if (RING_BUILD_TESTING)
    set(MESSAGE_PASSER_TESTS
            test/BalanceTests.cpp
            test/MessagePasserStatusTests.cpp
            test/AsyncSends_tests.cpp
            test/ParallelSumTests.cpp
            test/SendRecvScalarTests.cpp
            test/SendRecvVectorTests.cpp
            test/SendRecvVectorOfVectorsTests.cpp
            test/GatherTests.cpp
            test/BroadcastTests.cpp
            test/MagicTypeTests.cpp
            test/ScatterTests.cpp
            test/ScattervTests.cpp
            test/ReductionTests.cpp
            test/MessagePasserBasicsTests.cpp
            test/MessageTests.cpp
            test/ProbeTests.cpp
            test/SumAtIdTests.cpp
            test/SelfSend_test.cpp
            test/ExchangeClosureTests.cpp
            test/MessagePasserAllToAllTests.cpp
            test/FinalizeTests.cpp)
    add_catch_unit_test(MessagePasserTests
            ${MESSAGE_PASSER_TESTS}
            ${MESSAGE_PASSER_HEADERS}
            )

    target_link_libraries(MessagePasserTests PRIVATE MessagePasser::MessagePasser)
    target_compile_options(MessagePasserTests PRIVATE
            $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -Werror -Wunused -Wuninitialized -pedantic-errors
            -Wno-unused-parameter -Wno-sign-compare -Wno-cast-function-type -Wno-deprecated-copy>
            $<$<CXX_COMPILER_ID:Clang>: -Wall -Wextra -Werror -Wunused -Wuninitialized -pedantic-errors
            -Wno-unused-parameter -Wno-sign-compare -Wno-return-type-c-linkage -Wno-deprecated-copy>
            )
    if(CMAKE_CXX_COMPILER MATCHES nvcc_wrapper)
       target_compile_options(MessagePasserTests PRIVATE --host-only)
    endif()

    if (MPIEXEC_MAX_NUMPROCS)
        set(CORES_AVAILABLE ${MPIEXEC_MAX_NUMPROCS})
    else()
        include(ProcessorCount)
        ProcessorCount(CORES_AVAILABLE)
    endif()

    function(add_message_passer_test TARGET_NAME NUMBER_OF_MPI_PROCESSES)
        if(${CORES_AVAILABLE} GREATER_EQUAL ${NUMBER_OF_MPI_PROCESSES})
            set(TEST_NAME ${TARGET_NAME}-${NUMBER_OF_MPI_PROCESSES}-ranks)
            set (test_parameters -np ${NUMBER_OF_MPI_PROCESSES} "./${TARGET_NAME}")
            add_test(NAME ${TEST_NAME} COMMAND "mpirun" ${test_parameters})
            set_tests_properties(${TEST_NAME} PROPERTIES PROCESSORS ${NUMBER_OF_MPI_PROCESSES})
        endif()
    endfunction()

    add_message_passer_test(MessagePasserTests 2)
    add_message_passer_test(MessagePasserTests 3)
    add_message_passer_test(MessagePasserTests 4)
    add_message_passer_test(MessagePasserTests 5)
    add_message_passer_test(MessagePasserTests 6)
endif ()

add_library(MessagePasser INTERFACE)
add_library(MessagePasser::MessagePasser ALIAS MessagePasser)
target_include_directories(MessagePasser INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
        ${MPI_INCLUDE_PATH}
        )
target_link_libraries(MessagePasser INTERFACE ${MPI_LIBRARIES})

install(TARGETS MessagePasser
        EXPORT MessagePasser
        LIBRARY DESTINATION lib)

install(FILES ${MESSAGE_PASSER_HEADERS} DESTINATION include/MessagePasser)

install(EXPORT MessagePasser
        NAMESPACE MessagePasser:: DESTINATION cmake)

install(FILES MessagePasserConfig.cmake
        DESTINATION cmake)
