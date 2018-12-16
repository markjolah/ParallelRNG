# ParallelRngManager - C++ Parallel RNG Interface for OpenMP using TRNG
#
# Mark J. Olah (mjo@cs.unm DOT edu)
# Copyright2017-2018
# Licensed under the Apache License, Version 2.0
# https://www.apache.org/licenses/LICENSE-2.0
# See: LICENCE file
cmake_minimum_required( VERSION 3.9 )
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

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR}/cmake/UncommonCmakeModules)

### Dependencies
#OpenMP
find_package(OpenMP REQUIRED)

# Armadillo
find_package(Armadillo REQUIRED)
add_definitions(-DARMA_USE_CXX11)
add_definitions(-DARMA_DONT_USE_LAPACK -DARMA_DONT_USE_BLAS -DARMA_DONT_USE_OPENMP -DARMA_DONT_USE_HDF5) #disable unused dependencies
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:ARMA_PRINT_ERRORS>)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<NOT:$<CONFIG:Debug>>:ARMA_NO_DEBUG>)

### External dependencies
#TRNG (Tina's Random Number Generator) 
# Allows for ordered parallel sampling from a single RNG stream.
# TRNG packages do not exists for most linux distributions, so we are including 
# a FindTRNG.cmake and using AddExternalAutotoolsDependency to configure build and install
# trng to CMAKE_INSTALL_PREFIX at configure time.
include(AddExternalAutotoolsDependency)
set(DEPENDENT_LIBRARY_TYPES SHARED)
if(BUILD_STATIC_LIBS)
    list(APPEND DEPENDENT_LIBRARY_TYPES STATIC)
endif()

add_external_autotools_dependency(NAME TRNG URL "https://github.com/rabauke/trng4.git" ${DEPENDENT_LIBRARY_TYPES})

### GLOBAL compiler settings.
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:PARALLEL_RNG_DEBUG>) # flag to enable debug_assert
include(ConfigureDebugBuilds)

### PackageConfig
include(ExportPackageWizzard)
#Give path to FindTRNG.cmake so it can be called by PackageConfig.cmake correctly from build tree or install tree
export_package_wizzard(FIND_MODULES "cmake/UncommonCmakeModules/FindTRNG.cmake")

### Source directories
add_subdirectory(src)

### Testing
if(BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()
