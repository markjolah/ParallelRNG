# ParallelRngManager - C++ Parallel RNG Interface for OpenMP using TRNG
#
# Mark J. Olah (mjo@cs.unm DOT edu)
# Copyright 2017
# Licensed under the Apache License, Version 2.0
# https://www.apache.org/licenses/LICENSE-2.0
# See: LICENCE file

cmake_minimum_required( VERSION 3.1 )
project(ParallelRngManager VERSION 0.3 LANGUAGES CXX)
# cmake_policy(SET CMP0054 NEW) #Don't derrefernce in if() statements
# cmake_policy(SET CMP0022 NEW) #LINK_INTERFACE_LIBRARIES fix

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    option(BUILD_TESTING "Build testing framework" ON)
else()
    option(BUILD_TESTING "Build testing framework" OFF)
endif()
message(STATUS "OPTION: BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
message(STATUS "OPTION: BUILD_STATIC_LIBS: ${BUILD_STATIC_LIBS}")
message(STATUS "OPTION: BUILD_TESTING: ${BUILD_TESTING}")


if (NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
  set (BUILD_STATIC_LIBS ON)
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules 
                                           ${CMAKE_CURRENT_LIST_DIR}/cmake/Functions)

include(PackageConfig) #Configure Package-Config and associated files for cmake exports

### Dependencies
find_package(OpenMP REQUIRED)

# Armadillo
find_package(Armadillo REQUIRED)
add_definitions(-DARMA_USE_CXX11)
#disable armadillo numerical dependencies which are not used in this library
add_definitions(-DARMA_DONT_USE_WRAPPER)
add_definitions(-DARMA_DONT_USE_LAPACK)
add_definitions(-DARMA_DONT_USE_BLAS) 
add_definitions(-DARMA_DONT_USE_OPENMP)
add_definitions(-DARMA_DONT_USE_HDF5)
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_definitions(-DARMA_PRINT_ERRORS)
endif()

#External dependencies
#TRNG (Tina's Random Number Generator) 
# Allows for ordered parallel sampling from a single RNG stream.
# TRNG packages do not exists for most linux distributions, so we are including 
# a FindTRNG.cmake and using AddExternalAutotoolsDependency to configure build and install
# trng to CMAKE_INSTALL_PREFIX at configure time.
include(AddExternalAutotoolsDependency)
set(TRNG_GIT_URL https://github.com/rabauke/trng4.git)
AddExternalAutotoolsDependency(TRNG ${TRNG_GIT_URL} SHARED STATIC)


# if (WIN32)
#     add_definitions( -DWIN32 )
# elseif(UNIX AND NOT APPLE)
#     add_definitions( -DLINUX )
# endif()

### GLOBAL compiler settings.
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_compile_options(-W -Wall -Wextra)
    add_definitions(-DDEBUG)
elseif()
    add_definitions(-DNDEBUG)
endif()

set(CMAKE_DEBUG_POSTFIX ".debug" CACHE STRING "Debug file extension")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2")

### Source directories
add_subdirectory(src)
if(${BUILD_TESTING})
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()

install(FILES cmake/Modules/FindTRNG.cmake DESTINATION ${CONFIG_INSTALL_DIR} COMPONENT Development)
