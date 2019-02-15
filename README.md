<!--[![Build Status](https://travis-ci.org/markjolah/ParallelRngManager.svg?branch=master)](https://travis-ci.org/markjolah/ParallelRngManager)-->
<a href="https://travis-ci.org/markjolah/ParallelRngManager"><img src="https://travis-ci.org/markjolah/ParallelRngManager.svg?branch=master"/></a>
# Parallel RNG Manager

A ParallelRngManager simplifies the task of initializing and coordinating random number generation for multiple threads in OpenMP and other multi-threaded programming environments without the need for locks or the possibility of false sharing.  A single integer value is used to seed a single random number generator that is partitioned into independent parallel random number generator streams.

Using a single random number generator seed makes deterministic testing and debugging of parallel stochastic algorithms practical.  Additionally it is important to use a random number generator specifically designed for parallel use, as it is not in general safe to use independent random seeds for each processor if strong randomness properties and guaranteed a-correlation of the streams are arithmetically important considerations.

More generally, a _parallel random number generator_ (PRNG) provides a set of N random number generator streams for multi-threaded applications, where each stream is produced from a single underlying random number generator with a single global seed.  For certain classes of random number generators, a single stream can efficiently be partitioned into N threads without communication overhead. The [`parallel_rng::ParallelRngManager`](https://markjolah.github.io/ParallelRngManager/classparallel__rng_1_1ParallelRngManager.html) class functions as an OpenMP-aware manager for the PRNGs from the [Tina's Random Number Generator (TRNG) Library](https://www.numbercrunch.de/trng/).

## Features
 * *ParallelRngManager* is CMake based, and provides `ParallelRngManagerConfig.cmake` files allowing `find_pacakge(ParallelRngManager)` to find the package in either the build or install trees.
 * *ParallelRngManager* can automatically configure and install TRNG and alongside itself if it does not exist on the system.
 * *ParallelRngManager* is designed to work seamlessly with OpenMP.  It automatically manages the number of RNG streams based on hardware concurrency and prevents false sharing.

 * A *ParallelRngManager* object manages a single stream and uses OpenMP `get_num_threads()` to  allocate the correct number of sub-streams, which are kept on separate cache lines using [`aligned_array::AArray<RngT>`](https://github.com/markjolah/AlignedArray).

## Documentation
The ParallelRngManager Doxygen documentation can be build with the `OPT_DOC` CMake option and is also available on online:
  * [ParallelRngManager HTML Manual](https://markjolah.github.io/ParallelRngManager/index.html)
  * [ParallelRngManager PDF Manual](https://markjolah.github.io/ParallelRngManager/pdf/ParallelRngManager-0.3-reference.pdf)

## Installation

The easiest method is to use the default build script, which can be easily customized.  The default build directory is `./_build` and the default install directory is `./_install`.

    > git clone https://github.com/markjolah/ParallelRngManager.git
    > cd ParallelRngManager
    > ./build.sh

If TRNG is not available on the system, it is important to have `CMAKE_INSTALL_PREFIX` set to a valid install directory, even if it is just a local directory, as the autotools build is designed to install into the `CMAKE_INSTALL_PREFIX` and ParallelRngManager is then expecting to find the TRNG library there.

## CMake options
The following CMake options control the build.
 * `BUILD_SHARED_LIBS` - Build shared libraries
 * `BUILD_STATIC_LIBS` - Build static libraries
 * `BUILD_TESTING` - Build testing framework
 * `OPT_DOC` - Build documentation
 * `OPT_INSTALL_TESTING` - Install testing executables in install-tree.
 * `OPT_EXPORT_BUILD_TREE` - Configure the package so it is usable from the build tree.  Useful for development.
 * `OPT_ARMADILLO_INT64` - Use 64-bit integers for Armadillo, BLAS, and LAPACK.

## Dependencies

ParallelRngManager is designed to be portable, but relies on several system development and numerical libraries.
Currently Travis CI uses the *trusty* image to test ParallelRngManager
Standard system dependencies
 * *>=g++-4.9* - A `--std=c++11` compliant GCC compiler
 * *>=CMake-3.9*
 * [*OpenMP*](https://www.openmp.org/)
 * [*Armadillo*](http://arma.sourceforge.net/docs.html) - A high-performance array library for C++.
 * [*googletest*](https://github.com/google/googletest) - Required for testing (`BUILD_TESTING=On`)
 * [*Doxygen*](https://github.com/google/googletest) - Required to generate documentation (`OPT_DOC=On`)
    * *graphviz* - Required to generate documentation (`make doc`)
    * *LAPACK* - Required for generate pdf documenation (`make pdf`)

### Tina's Random Number Generator (TRNG)

The ParallelRngManager is a lightweight wrapper around the [Tina's Random Number Generator (TRNG)](https://www.numbercrunch.de/trng/) library.  This rather specialized numerical library is normally not available on most Linux distributions, so for convenience the ParallelRngManager CMake build system will automatically download, configure, build, and install TRNG (`libtrng4.so`) into the `CMAKE_INSTALL_PREFIX` path if it is not already present on the build system.  This process uses the `AddExternalAutotoolsDependency.cmake` function from the [UncommonCMakeModules](https://github.com/markjolah/UncommonCMakeModules) dependency.

  * [TRNG Manual](https://www.numbercrunch.de/trng/trng.pdf)
  * [H. Bauke and S. Mertens.  _Random Numbers for Large Scale Distributed Monte Carlo Simulations_.](http://arxiv.org/abs/cond-mat/0609584)

### Other dependencies
ParallelRngManager uses these reusable header-only component libraries via  [`git subrepo`](https://github.com/ingydotnet/git-subrepo)
  *  [AlignedArray](https://github.com/markjolah/AlignedArray) - Provides `aligned_array::AArray<T>` which is an STL conforming fixed-length array container which guarantees no two elements share a cache line, preventing false sharing in multi-threaded or OpenMP programs.  ParallelRngManager stores RNG streams in an `AArray<RngT>` array to prevent false sharing.
  *  [AnyRng](https://github.com/markjolah/AnyRng) - Provides `any_rng::AnyRng<result_type>` which is a type-erased STL random number generator type.
  *  [UncommonCMakeModules](https://github.com/markjolah/UncommonCMakeModules) - Provides `FindTRNG.cmake` `FindArmadillo.cmake` and other useful CMake functions like `ExportPackageWizzard.cmake`.  ParallelRngManager only uses a small portion of these CMake modules but using a `git subrepo` pulls in the entire repository.



## Testing
ParallelRngManager uses [googletest](https://github.com/google/googletest) for C++ unit testing and integrates with CTest.  To build tests, enable the `BUILD_TESTING` CMake option and possibly also the `OPT_INSTALL_TESTING` option to install tests along with ParallelRngManager.

Tests can be run with:

    > make test
