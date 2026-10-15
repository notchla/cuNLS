cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
  set(_cunls_default_cuda_architectures ON)
endif()
if(NOT DEFINED CMAKE_CUDA_COMPILER AND NOT DEFINED ENV{CUDACXX} AND EXISTS "/usr/local/cuda/bin/nvcc")
  set(CMAKE_CUDA_COMPILER "/usr/local/cuda/bin/nvcc")
endif()

project(cunls_install_test LANGUAGES CXX CUDA)

# Same default as the cunls build: Blackwell cubins need CUDA 12.8.
if(_cunls_default_cuda_architectures)
  set(CMAKE_CUDA_ARCHITECTURES 75 80 86 89)
  if(CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL 12.8)
    list(APPEND CMAKE_CUDA_ARCHITECTURES 100 120)
  endif()
  set(CMAKE_CUDA_ARCHITECTURES "${CMAKE_CUDA_ARCHITECTURES}" CACHE STRING "CUDA architectures to compile" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

find_package(cunls REQUIRED)

add_executable(install_test main.cpp)
target_link_libraries(install_test PRIVATE cunls::cunls)
