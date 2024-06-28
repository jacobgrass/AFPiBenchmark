cmake_minimum_required(VERSION 3.12)

project(AFPiBenchMark)

# Use CMAKE_PREFIX_PATH to help find the installed ArrayFire package
list(APPEND CMAKE_PREFIX_PATH "$ENV{CMAKE_PREFIX_PATH}")

#add_executable(AFPiBenchMark "main.cpp" "AFPiBenchMark.h")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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
