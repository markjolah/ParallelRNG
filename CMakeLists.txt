# ParallelRngManager - C++ Parallel RNG Interface for OpenMP using TRNG
#
# Mark J. Olah (mjo@cs.unm DOT edu)
# Copyright2017-2018
# Licensed under the Apache License, Version 2.0
# https://www.apache.org/licenses/LICENSE-2.0
# See: LICENCE file


cmake_minimum_required( VERSION 3.8 )
project(ParallelRngManager VERSION 0.3 LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    option(BUILD_TESTING "Build testing framework" ON)
else()
    option(BUILD_TESTING "Build testing framework" OFF)
endif()

if (NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
  set (BUILD_STATIC_LIBS ON)
endif ()

message(STATUS "OPTION: BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
message(STATUS "OPTION: BUILD_STATIC_LIBS: ${BUILD_STATIC_LIBS}")
message(STATUS "OPTION: BUILD_TESTING: ${BUILD_TESTING}")


list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules 
                                ${CMAKE_CURRENT_LIST_DIR}/cmake/Functions)

### Dependencies
find_package(OpenMP REQUIRED)

# Armadillo
find_package(Armadillo REQUIRED)
add_definitions(-DARMA_USE_CXX11)
add_definitions(-DARMA_DONT_USE_LAPACK -DARMA_DONT_USE_BLAS -DARMA_DONT_USE_OPENMP -DARMA_DONT_USE_HDF5) #disable unused dependencies
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:ARMA_PRINT_ERRORS>)

#External dependencies
#TRNG (Tina's Random Number Generator) 
# Allows for ordered parallel sampling from a single RNG stream.
# TRNG packages do not exists for most linux distributions, so we are including 
# a FindTRNG.cmake and using AddExternalAutotoolsDependency to configure build and install
# trng to CMAKE_INSTALL_PREFIX at configure time.
include(AddExternalAutotoolsDependency)
set(TRNG_GIT_URL https://github.com/rabauke/trng4.git)
if(BUILD_SHARED_LIBS)
    list(APPEND EXTERN_BUILD_TYPES SHARED)
endif()
if(BUILD_STATIC_LIBS)
    list(APPEND EXTERN_BUILD_TYPES STATIC)
endif()

AddExternalAutotoolsDependency(TRNG ${TRNG_GIT_URL} ${EXTERN_BUILD_TYPES})

### GLOBAL compiler settings.
add_compile_options($<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-O0>)
add_compile_options($<$<CONFIG:Debug>:-W>)
add_compile_options($<$<CONFIG:Debug>:-Wall>)
add_compile_options($<$<CONFIG:Debug>:-Wextra>)

set(CMAKE_DEBUG_POSTFIX ".debug" CACHE STRING "Debug file extension")

### PackageConfig
include(ExportPackageWizzard)
export_package_wizzard(FIND_MODULES "cmake/Modules/FindTRNG.cmake")

### Source directories
add_subdirectory(src)


### Testing
if(${BUILD_TESTING})
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()

### Installation
#Find module for TRNG is provided
