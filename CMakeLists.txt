# CMakeList.txt : CMake project for AFPiBenchMark, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (AFPiBenchMark "AFPiBenchMark.cpp" "AFPiBenchMark.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AFPiBenchMark PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

find_package(ArrayFire)
# Find OpenMP
find_package(OpenMP REQUIRED)
if(ArrayFire_CPU_FOUND)
  add_executable(AFPiBenchMark_cpu AFPiBenchMark.cpp)
  target_link_libraries(AFPiBenchMark_cpu ArrayFire::afcpu)
endif()

if(ArrayFire_CUDA_FOUND)
  add_executable(AFPiBenchMark_cuda AFPiBenchMark.cpp)
  target_link_libraries(AFPiBenchMark_cuda ArrayFire::afcuda)
endif()

if(ArrayFire_OpenCL_FOUND)
  add_executable(AFPiBenchMark_opencl AFPiBenchMark.cpp)
  target_link_libraries(AFPiBenchMark_opencl ArrayFire::afopencl)
endif()

if(ArrayFire_oneAAFPiBenchMark_FOUND)
  add_executable(AFPiBenchMark_oneaAFPiBenchMark AFPiBenchMark.cpp)
  target_link_libraries(AFPiBenchMark_oneaAFPiBenchMark ArrayFire::afoneaAFPiBenchMark)
endif()

