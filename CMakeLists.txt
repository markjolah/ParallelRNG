# ParallelRngManager - C++ Parallel RNG Interface for OpenMP using TRNG
#
# Mark J. Olah (mjo@cs.unm DOT edu)
# Copyright 2017-2019
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
option(OPT_INSTALL_TESTING "Install testing executables" OFF)
option(OPT_EXPORT_BUILD_TREE "Configure the package so it is usable from the build tree.  Useful for development." OFF)
option(OPT_ARMADILLO_INT64 "Use 64-bit integers for Armadillo, BLAS, and LAPACK." OFF)

if(NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
  set (BUILD_SHARED_LIBS ON)  #Must build at least one of SHARED_ and STATIC_LIBS.  Default SHARED_
endif()

message(STATUS "OPTION: BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
message(STATUS "OPTION: BUILD_STATIC_LIBS: ${BUILD_STATIC_LIBS}")
message(STATUS "OPTION: BUILD_TESTING: ${BUILD_TESTING}")
message(STATUS "OPTION: OPT_INSTALL_TESTING: ${OPT_INSTALL_TESTING}")
message(STATUS "OPTION: OPT_EXPORT_BUILD_TREE: ${OPT_EXPORT_BUILD_TREE}")
message(STATUS "OPTION: OPT_ARMADILLO_INT64: ${OPT_ARMADILLO_INT64}")

#Add UcommonCmakeModules git subpreo to path.
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR}/cmake/UncommonCMakeModules)

### Dependencies
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

#OpenMP
find_package(OpenMP REQUIRED)
find_package(Pthread REQUIRED)

# Find Blas and Lapack using pkg-config in cross-compilation aware way.  Using local FindBLAS.cmake and FindLAPACK.cmake.
if(OPT_ARMADILLO_INT64)
    set(ARMADILLO_INT64_COMPONENT INT64) #Enable INT64 interface for armadillo, Blas and Lapack
else()
    set(ARMADILLO_INT64_COMPONENT)
endif()
find_package(Armadillo REQUIRED COMPONENTS CXX11 ${ARMADILLO_INT64_COMPONENT})
add_compile_definitions(${ARMADILLO_PRIVATE_COMPILE_DEFINITIONS})


### GLOBAL compiler settings.
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:PARALLEL_RNG_DEBUG>) # flag to enable debug_assert
include(ConfigureDebugBuilds)

### PackageConfig
include(ExportPackageWizzard)
set(EXPORTED_FIND_MODULES FindArmadillo.cmake FindTRNG.cmake)
list(TRANSFORM EXPORTED_FIND_MODULES PREPEND cmake/UncommonCMakeModules/)
export_package_wizzard(PROVIDED_COMPONENTS ${ARMADILLO_INT64_COMPONENT}
                       FIND_MODULES ${EXPORTED_FIND_MODULES})

### Source directories
add_subdirectory(src)

### Testing
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
