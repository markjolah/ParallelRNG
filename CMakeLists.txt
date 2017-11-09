# ParallelRngManager - C++ Parallel RNG Interface for OpenMP using TRNG
#
# Mark J. Olah (mjo@cs.unm DOT edu)
# Copyright 2017
# Licensed under the Apache License, Version 2.0
# https://www.apache.org/licenses/LICENSE-2.0
# See: LICENCE file

cmake_minimum_required( VERSION 3.0 )
project(ParallelRngManager VERSION 0.1 LANGUAGES CXX)

#Add to /cmake/Modules to modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

# Armadillo
find_package(Armadillo REQUIRED)
add_definitions(-DARMA_USE_CXX11 -DARMA_DONT_USE_WRAPPER -DARMA_BLAS_LONG)
add_definitions(-DARMA_DONT_USE_LAPACK) #Want to explicitly use openMP when required
add_definitions(-DARMA_DONT_USE_BLAS) #Want to explicitly use openMP when required
add_definitions(-DARMA_DONT_USE_OPENMP) #Want to explicitly use openMP when required
add_definitions(-DARMA_DONT_USE_HDF5) #Causes problems and not necessary
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_definitions(-DARMA_PRINT_ERRORS)
endif()

#TRNG Tina's Random Number Generator
find_library(TRNG_LIBRARY trng4)

# Compiler Definitions
if (WIN32)
    add_definitions( -DWIN32 )
elseif(UNIX AND NOT APPLE)
    add_definitions( -DLINUX )
endif()

set(GCC_WARN_FLAGS "-W -Wall -Wextra -Werror -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_WARN_FLAGS} ${GCC_STD_FLAGS}")

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_definitions(-DDEBUG)
else()
    add_definitions(-DNDEBUG)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#Add debug postfix to libraries built in debug mode
set(CMAKE_DEBUG_POSTFIX ".debug" CACHE STRING "Debug file extension")

include(PackageConfig) #Configure Package-Config and associated files for cmake exports

add_subdirectory(src)
