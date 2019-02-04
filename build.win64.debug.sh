#!/bin/bash
#
# build.win64.debug.sh
#
#
ARCH=win64
FULL_ARCH=x86_64-w64-mingw32
TOOLCHAIN_FILE=./cmake/UncommonCMakeModules/Toolchains/Toolchain-MXE-${FULL_ARCH}.cmake
INSTALL_PATH=_${ARCH}.install
BUILD_PATH=_${ARCH}.build/Debug
NUM_PROCS=`grep -c ^processor /proc/cpuinfo`

ARGS="-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE"
ARGS="${ARGS} -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH"
ARGS="${ARGS} -DBUILD_TESTING=On"
ARGS="${ARGS} -DBUILD_STATIC_LIBS=ON"
ARGS="${ARGS} -DBUILD_SHARED_LIBS=ON"
ARGS="${ARGS} -DBUILD_TESTING=On"
ARGS="${ARGS} -DOPT_INSTALL_TESTING=On"
ARGS="${ARGS} -DOPT_EXPORT_BUILD_TREE=On"
ARGS="${ARGS} -DOPT_FIXUP_DEPENDENCIES=On"
ARGS="${ARGS} -DOPT_FIXUP_DEPENDENCIES_BUILD_TREE=On"

set -ex
rm -rf $INSTALL_PATH $BUILD_PATH
cmake -H. -B$BUILD_PATH  -DCMAKE_BUILD_TYPE=Debug ${ARGS}
VERBOSE=1 cmake --build $BUILD_PATH --target install -- -j${NUM_PROCS}