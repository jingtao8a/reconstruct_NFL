cmake_minimum_required(VERSION 3.12)
project(reconstruct_NFL VERSION 1.0 LANGUAGES  C CXX)

# specify c/c++ standard
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif(NOT CMAKE_CXX_STANDARD)

if (MSVC)
  set(CMAKE_CXX_FLAGS "/O2 /arch:AVX2 /W1 /EHsc")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "-O3 -xHost -ggdb3 -pthread")
else ()
  set(CMAKE_CXX_FLAGS "-O3 -march=native -ggdb3 -pthread")
endif ()

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(LIB_DIR "${PROJECT_SOURCE_DIR}/lib")

include_directories(${SRC_DIR} ${LIB_DIR} competitor)
add_subdirectory(test)

add_executable(format "${SRC_DIR}/util/format_data.cc")
add_executable(data_gen "${SRC_DIR}/util/data_generator.cc")
add_executable(workload_gen "${SRC_DIR}/util/workload_generator.cc")
add_executable(nf_convert "${SRC_DIR}/util/nf_data_converter.cc")
add_executable(benchmark "${SRC_DIR}/benchmark/benchmark.cc")
add_executable(torch_modules_test "${SRC_DIR}/torch_models/torch_models_test.cpp")

set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)
set(Torch_DIR /home/chengang/anaconda3/envs/jingtao8a_env/lib/python3.8/site-packages/torch/share/cmake/Torch)
find_package(PythonInterp REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development NumPy)
find_package(Torch REQUIRED)
if (TORCH_FOUND)
  include_directories(${TORCH_INCLUDE_DIRS})
  target_link_libraries(benchmark ${TORCH_LIBRARIES} Python3::Python Python3::NumPy)
  target_link_libraries(torch_modules_test ${TORCH_LIBRARIES})
elseif ()
  message(WARNING "torch libs not found")
endif()

list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/lib/libmkl")
find_package(MKL)
if (MKL_FOUND)
  include_directories(${MKL_INCLUDE_DIR})
  target_link_libraries(benchmark ${MKL_LIBRARIES})
else ()
  message(WARNING "MKL libs not found")
endif ()

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else ()
  message(WARNING "OpenMP libs not found")
endif ()




