cmake_minimum_required(VERSION 3.12)

project(AFPiBenchMark)

# Use CMAKE_PREFIX_PATH to help find the installed ArrayFire package
list(APPEND CMAKE_PREFIX_PATH "$ENV{CMAKE_PREFIX_PATH}")

#add_executable(AFPiBenchMark "main.cpp" "AFPiBenchMark.h")

# Build optimised by default: without a build type, single-config generators
# pass no -O flag to the compiler
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Record the compiler, build type and flags in each results folder
string(TOUPPER "${CMAKE_BUILD_TYPE}" AFPI_BUILD_TYPE_UPPER)
add_compile_definitions(AFPI_BUILD_INFO="${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, build type ${CMAKE_BUILD_TYPE}, flags ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${AFPI_BUILD_TYPE_UPPER}}")

# Find ArrayFire
find_package(ArrayFire REQUIRED)
# Find OpenMP
find_package(OpenMP REQUIRED)

# Compile and link each variant of the benchmark, if available
if(ArrayFire_CPU_FOUND)
  add_executable(AFPiBenchMark_cpu main.cpp BenchmarkTest.h BenchmarkTest.cpp)
  target_link_libraries(AFPiBenchMark_cpu ArrayFire::afcpu OpenMP::OpenMP_CXX)
  add_custom_command(TARGET AFPiBenchMark_cpu POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ArrayFire::afcpu> $<TARGET_FILE_DIR:AFPiBenchMark_cpu>)
endif()

if(ArrayFire_CUDA_FOUND)
  add_executable(AFPiBenchMark_cuda main.cpp BenchmarkTest.h BenchmarkTest.cpp)
  target_link_libraries(AFPiBenchMark_cuda ArrayFire::afcuda OpenMP::OpenMP_CXX)
  add_custom_command(TARGET AFPiBenchMark_cuda POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ArrayFire::afcuda> $<TARGET_FILE_DIR:AFPiBenchMark_cuda>)
endif()

if(ArrayFire_OpenCL_FOUND)
  add_executable(AFPiBenchMark_opencl main.cpp BenchmarkTest.h BenchmarkTest.cpp)
  target_link_libraries(AFPiBenchMark_opencl ArrayFire::afopencl OpenMP::OpenMP_CXX)
  add_custom_command(TARGET AFPiBenchMark_opencl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ArrayFire::afopencl> $<TARGET_FILE_DIR:AFPiBenchMark_opencl>)
endif()
