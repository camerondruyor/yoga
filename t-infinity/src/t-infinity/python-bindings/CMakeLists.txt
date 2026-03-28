cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

find_package(pybind11 REQUIRED)

pybind11_add_module(_infinity_plugins SHARED
        PybindIncludes.h
        PythonBindings.cpp
        PythonMotionMatrixBindings.cpp
        PythonFilterBindings.cpp
        PythonVisualizationBindings.cpp
        PythonMeshBindings.cpp
        PythonFieldBindings.cpp
        PythonFluidSolverBindings.cpp
        PythonAdaptationBindings.cpp
        PythonMPIBindings.cpp
        PythonDomainAssemblerBindings.cpp
        PythonSnapBindings.cpp
        PythonTracerBindings.cpp
        )
target_include_directories(_infinity_plugins PUBLIC
        ${PYTHON_INCLUDE_DIRS}
        )
target_link_libraries(_infinity_plugins
        PRIVATE
        ${PYTHON_LIBRARIES}
        infinity::infinity
        pybind11::module
        )
set_target_properties(_infinity_plugins PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Due to Pybind11 PYBIND11_EXPAND_SIDE_EFFECTS macro
target_compile_options(_infinity_plugins PRIVATE 
        $<$<CXX_COMPILER_ID:Clang>: -Wno-unused-value>
        $<$<CXX_COMPILER_ID:AppleClang>: -Wno-unused-value>
        $<$<CXX_COMPILER_ID:GNU>: -Wno-deprecated-declarations>
        )

if (CMAKE_CXX_COMPILER MATCHES nvcc_wrapper)
   # Nothing in the pybind11 is called from a CUDA kernel, 
   # so compile with the host compiler to avoid annoying nvcc warnings
   target_compile_options(_infinity_plugins PRIVATE --host-only)
endif()

set_target_rpath(_infinity_plugins ../../lib ../../lib64)
install(TARGETS _infinity_plugins
        EXPORT _infinity_plugins
        DESTINATION python/infinity_plugins
        RUNTIME DESTINATION python/infinity_plugins
        )
install(EXPORT _infinity_plugins
        NAMESPACE infinity:: DESTINATION cmake)

